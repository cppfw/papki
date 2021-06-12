#pragma once

#include <utki/config.hpp>

#include "file.hpp"

namespace papki{

//TODO: doxygen
class root_dir : public file{
	std::unique_ptr<file> base_file;
	std::string root_directory;
	
public:
	/**
	 * @param base_file - a file to wrap.
	 * @param root_directory - path to the root directory to set. It should have trailing '/' character.
	 */
	root_dir(std::unique_ptr<file> base_file, std::string_view root_directory) :
			base_file(std::move(base_file)),
			root_directory(root_directory)
	{
		if(!this->base_file){
			throw std::invalid_argument("root_dir(): passed in base file pointer is null");
		}
		this->file::set_path_internal(std::string(this->base_file->path()));
		this->base_file->set_path(this->root_directory + this->path());
	}
	
	static std::unique_ptr<const root_dir> make(std::unique_ptr<const file> base_file, const std::string& root_directory){
		return std::make_unique<const root_dir>(std::unique_ptr<file>(const_cast<file*>(base_file.release())), root_directory);
	}
	
	root_dir(const root_dir&) = delete;
	root_dir& operator=(const root_dir&) = delete;
	
private:
	void set_path_internal(std::string&& pathName)const override{
		this->file::set_path_internal(std::move(pathName));
		this->base_file->set_path(this->root_directory + this->path());
	}
	
	void open_internal(mode io_mode)override{
		this->base_file->open(io_mode);
	}
	
	void close_internal()const noexcept override{
		this->base_file->close();
	}
	
	std::vector<std::string> list_dir(size_t maxEntries = 0)const override{
		return this->base_file->list_dir(maxEntries);
	}
	
	size_t read_internal(utki::span<uint8_t> buf)const override{
		return this->base_file->read(buf);
	}
	
	size_t write_internal(utki::span<const uint8_t> buf)override{
		return this->base_file->write(buf);
	}
	
	size_t seek_forward_internal(size_t numBytesToSeek)const override{
		return this->base_file->seek_forward(numBytesToSeek);
	}
	
	size_t seek_backward_internal(size_t numBytesToSeek)const override{
		return this->base_file->seek_backward(numBytesToSeek);
	}
	
	void rewind_internal()const override{
		this->base_file->rewind();
	}
	
	void make_dir()override{
		this->base_file->make_dir();
	}
	
	bool exists()const override{
		return this->base_file->exists();
	}
	
	std::unique_ptr<file> spawn()override{
		return std::make_unique<root_dir>(this->base_file->spawn(), this->root_directory);
	}
};

}
