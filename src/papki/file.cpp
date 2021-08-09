/*
The MIT License (MIT)

Copyright (c) 2015-2021 Ivan Gagis <igagis@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/* ================ LICENSE END ================ */

#include <list>
#include <cstring>

#include "file.hpp"

using namespace papki;

bool file::is_dir()const noexcept{
	if(this->path().size() == 0){
		return false;
	}

	ASSERT(this->path().size() > 0)
	if(this->path().back() == '/'){
		return true;
	}

	return false;
}

std::vector<std::string> file::list_dir(size_t max_entries)const{
	throw std::runtime_error("file::list_dir(): not supported for this file instance");
}

size_t file::read(utki::span<uint8_t> buf)const{
	if(!this->is_open()){
		throw std::logic_error("Cannot read, file is not opened");
	}
	
	size_t ret = this->read_internal(buf);
	this->current_pos += ret;
	return ret;
}

size_t file::write(utki::span<const uint8_t> buf){
	if(!this->is_open()){
		throw std::logic_error("Cannot write, file is not opened");
	}

	if(this->io_mode != mode::write){
		throw std::logic_error("file is opened, but not in write mode");
	}
	
	size_t ret = this->write_internal(buf);
	this->current_pos += ret;
	return ret;
}

size_t file::seek_forward_internal(size_t numBytesToSeek)const{
	std::array<uint8_t, 0x1000> buf; // 4kb buffer
	
	size_t bytesRead = 0;
	for(; bytesRead != numBytesToSeek;){
		size_t curNumToRead = numBytesToSeek - bytesRead;
		curNumToRead = std::min(curNumToRead, buf.size()); // clamp top
		size_t res = this->read(utki::make_span(&*buf.begin(), curNumToRead));
		ASSERT(bytesRead < numBytesToSeek)
		ASSERT(numBytesToSeek >= res)
		ASSERT(bytesRead <= numBytesToSeek - res)
		bytesRead += res;
		
		if(res != curNumToRead){ // if end of file reached
			break;
		}
	}
	this->current_pos -= bytesRead; // make correction to curPos, since we were using read()
	return bytesRead;
}

void file::make_dir(){
	throw std::runtime_error("Make directory is not supported");
}

std::vector<uint8_t> file::load(size_t max_bytes_to_load)const{
	if(this->is_open()){
		throw std::logic_error("file::load(): file should not be opened");
	}

	std::vector<uint8_t> ret;

	file::guard file_guard(*this); // make sure we close the file upon exit from the function
	
	const size_t read_chunk_size = 0x1000; // 4kb

	for(size_t num_bytes_read = 0; max_bytes_to_load != 0; num_bytes_read += read_chunk_size){
		auto num_bytes_to_read = std::min(max_bytes_to_load, read_chunk_size);
		ret.resize(ret.size() + num_bytes_to_read);
		auto n = this->read(utki::make_span(&ret[num_bytes_read], num_bytes_to_read));
		ASSERT(n <= num_bytes_to_read)
		if(n != num_bytes_to_read){
			ret.resize(num_bytes_read + n);
			return ret;
		}
		ASSERT(max_bytes_to_load >= num_bytes_to_read)
		max_bytes_to_load -= num_bytes_to_read;
	}

	return ret;
}

bool file::exists()const{
	if(this->is_dir()){
		// TODO: implement checking for directory existance
		throw std::logic_error("file::exists(): path is a directory, checking for directory existence is not yet supported");
	}

	if(this->is_open()){
		return true;
	}

	// try opening and closing the file to find out if it exists or not
	ASSERT(!this->is_open())
	try{
		file::guard fileGuard(const_cast<file&>(*this), file::mode::read);
	}catch(std::runtime_error&){
		return false; // file opening failed, assume the file does not exist
	}
	return true; // file open succeeded => file exists
}

uint64_t file::size()const{
	if(this->is_open()){
		throw std::logic_error("file must not be open when calling file::size() method");
	}

	file::guard file_guard(*this, file::mode::read);

	return this->seek_forward(~0);
}

file::guard::guard(const file& f, mode io_mode) :
		f(f)
{
	if(this->f.is_open()){
		throw std::logic_error("file::guard::guard(): file is already opened");
	}

	const_cast<file&>(this->f).open(io_mode);
}

file::guard::guard(const file& f) :
		f(f)
{
	if(this->f.is_open()){
		throw std::logic_error("file::guard::guard(): file is already opened");
	}

	this->f.open();
}

file::guard::~guard(){
	this->f.close();
}
