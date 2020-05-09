#pragma once

#include <utki/config.hpp>

#include "file.hpp"

namespace papki{

//TODO: doxygen
class root_dir : public file{
	std::unique_ptr<file> baseFile;
	std::string rootDir;
	
public:
	/**
	 * @param baseFile - a file to wrap.
	 * @param rootDir - path to the root directory to set. It should have trailing '/' character.
	 */
	root_dir(std::unique_ptr<file> baseFile, const std::string& rootDir) :
			baseFile(std::move(baseFile)),
			rootDir(rootDir)
	{
		if(!this->baseFile){
			throw std::invalid_argument("root_dir(): passed in base file pointer is null");
		}
		this->file::set_path_internal(this->baseFile->path());
		this->baseFile->setPath(this->rootDir + this->path());
	}
	
	static std::unique_ptr<const root_dir> make(std::unique_ptr<const file> baseFile, const std::string& rootDir){
		return utki::make_unique<const root_dir>(std::unique_ptr<file>(const_cast<file*>(baseFile.release())), rootDir);
	}

	//TODO: deprecated, remove.
	static std::unique_ptr<const root_dir> makeUniqueConst(std::unique_ptr<const file> baseFile, const std::string& rootDir){
		return make(std::move(baseFile), rootDir);
	}
	
	root_dir(const root_dir&) = delete;
	root_dir& operator=(const root_dir&) = delete;
	
private:
	void set_path_internal(const std::string& pathName)const override{
		this->file::set_path_internal(pathName);
		this->baseFile->setPath(this->rootDir + pathName);
	}
	
	void open_internal(mode io_mode)override{
		this->baseFile->open(io_mode);
	}
	
	void close_internal()const noexcept override{
		this->baseFile->close();
	}
	
	std::vector<std::string> list_dir(size_t maxEntries = 0)const override{
		return this->baseFile->list_dir(maxEntries);
	}
	
	size_t read_internal(utki::span<uint8_t> buf)const override{
		return this->baseFile->read(buf);
	}
	
	size_t write_internal(const utki::span<uint8_t> buf)override{
		return this->baseFile->write(buf);
	}
	
	size_t seek_forward_internal(size_t numBytesToSeek)const override{
		return this->baseFile->seekForward(numBytesToSeek);
	}
	
	size_t seek_backward_internal(size_t numBytesToSeek)const override{
		return this->baseFile->seekBackward(numBytesToSeek);
	}
	
	void rewind_internal()const override{
		this->baseFile->rewind();
	}
	
	void make_dir()override{
		this->baseFile->make_dir();
	}
	
	bool exists()const override{
		return this->baseFile->exists();
	}
	
	std::unique_ptr<file> spawn()override{
		return utki::make_unique<root_dir>(this->baseFile->spawn(), this->rootDir);
	}
};

}
