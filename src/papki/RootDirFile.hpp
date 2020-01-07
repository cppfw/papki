/**
 * @file File wrapper allowing to set root path.
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <utki/config.hpp>

#include "file.hpp"

namespace papki{

//TODO: doxygen
class RootDirFile : public File{
	std::unique_ptr<File> baseFile;
	std::string rootDir;
	
public:
	/**
	 * @param baseFile - a File to wrap.
	 * @param rootDir - path to the root directory to set. It should have trailing '/' character.
	 */
	RootDirFile(std::unique_ptr<File> baseFile, const std::string& rootDir) :
			baseFile(std::move(baseFile)),
			rootDir(rootDir)
	{
		if(!this->baseFile){
			throw std::invalid_argument("RootDirFile(): passed in base file pointer is null");
		}
		this->file::set_path_internal(this->baseFile->path());
		this->baseFile->setPath(this->rootDir + this->path());
	}
	
	static std::unique_ptr<const RootDirFile> makeUniqueConst(std::unique_ptr<const File> baseFile, const std::string& rootDir){
		return utki::make_unique<const RootDirFile>(std::unique_ptr<File>(const_cast<File*>(baseFile.release())), rootDir);
	}
	
	RootDirFile(const RootDirFile&) = delete;
	RootDirFile& operator=(const RootDirFile&) = delete;
	
private:
	void set_path_internal(const std::string& pathName)const override{
		this->file::set_path_internal(pathName);
		this->baseFile->setPath(this->rootDir + pathName);
	}
	
	void open_internal(mode mode)override{
		this->baseFile->open(mode);
	}
	
	void close_internal()const noexcept override{
		this->baseFile->close();
	}
	
	std::vector<std::string> list_dir(size_t maxEntries = 0)const override{
		return this->baseFile->list_dir(maxEntries);
	}
	
	size_t readInternal(utki::span<std::uint8_t> buf)const override{
		return this->baseFile->read(buf);
	}
	
	size_t writeInternal(const utki::span<std::uint8_t> buf)override{
		return this->baseFile->write(buf);
	}
	
	size_t seekForwardInternal(size_t numBytesToSeek)const override{
		return this->baseFile->seekForward(numBytesToSeek);
	}
	
	size_t seekBackwardInternal(size_t numBytesToSeek)const override{
		return this->baseFile->seekBackward(numBytesToSeek);
	}
	
	void rewindInternal()const override{
		this->baseFile->rewind();
	}
	
	void makeDir()override{
		this->baseFile->makeDir();
	}
	
	bool exists()const override{
		return this->baseFile->exists();
	}
	
	std::unique_ptr<File> spawn()override{
		return utki::make_unique<RootDirFile>(this->baseFile->spawn(), this->rootDir);
	}
};

}
