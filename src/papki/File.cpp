/* The MIT License:

Copyright (c) 2009-2014 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

// Home page: http://ting.googlecode.com



#include <list>

#include "File.hpp"

#include "../util.hpp"



using namespace ting::fs;



std::string File::Ext()const{
	size_t dotPos = this->Path().rfind('.');
	if(dotPos == std::string::npos || dotPos == 0){//NOTE: dotPos is 0 for hidden files in *nix systems
		return std::string();
	}else{
		ASSERT(dotPos > 0)
		ASSERT(this->Path().size() > 0)
		ASSERT(this->Path().size() >= dotPos + 1)
		
		//Check for hidden file on *nix systems
		if(this->Path()[dotPos - 1] == '/'){
			return std::string();
		}
		
		return std::string(this->Path(), dotPos + 1, this->Path().size() - (dotPos + 1));
	}
	ASSERT(false)
}



std::string File::Dir()const{
	size_t slashPos = this->Path().rfind('/');
	if(slashPos == std::string::npos){//no slash found
		return std::string();
	}

	ASSERT(slashPos > 0)
	ASSERT(this->Path().size() > 0)
	ASSERT(this->Path().size() >= slashPos + 1)

	return std::string(this->Path(), 0, slashPos + 1);
}



std::string File::NotDir()const{
	size_t slashPos = this->Path().rfind('/');
	if(slashPos == std::string::npos){//no slash found
		return this->Path();
	}

	ASSERT(slashPos > 0)
	ASSERT(this->Path().size() > 0)
	ASSERT(this->Path().size() >= slashPos + 1)

	return std::string(this->Path(), slashPos + 1);
}



bool File::IsDir()const noexcept{
	if(this->Path().size() == 0){
		return false;
	}

	ASSERT(this->Path().size() > 0)
	if(this->Path()[this->Path().size() - 1] == '/'){
		return true;
	}

	return false;
}



std::vector<std::string> File::ListDirContents(size_t maxEntries)const{
	throw File::Exc("File::ListDirContents(): not supported for this File instance");
}



size_t File::Read(ting::Buffer<std::uint8_t> buf)const{
	if(!this->IsOpened()){
		throw File::IllegalStateExc("Cannot read, file is not opened");
	}
	
	size_t ret = this->ReadInternal(buf);
	this->curPos += ret;
	return ret;
}



size_t File::Write(ting::Buffer<const std::uint8_t> buf){
	if(!this->IsOpened()){
		throw File::IllegalStateExc("Cannot write, file is not opened");
	}

	if(this->ioMode != E_Mode::WRITE){
		throw File::Exc("file is opened, but not in WRITE mode");
	}
	
	size_t ret = this->WriteInternal(buf);
	this->curPos += ret;
	return ret;
}



size_t File::SeekForwardInternal(size_t numBytesToSeek)const{
	std::array<std::uint8_t, 0x1000> buf;//4kb buffer
	
	size_t bytesRead = 0;
	for(; bytesRead != numBytesToSeek;){
		size_t curNumToRead = numBytesToSeek - bytesRead;
		ting::util::ClampTop(curNumToRead, buf.size());
		size_t res = this->Read(ting::Buffer<std::uint8_t>(&*buf.begin(), curNumToRead));
		ASSERT(bytesRead < numBytesToSeek)
		ASSERT(numBytesToSeek >= res)
		ASSERT(bytesRead <= numBytesToSeek - res)
		bytesRead += res;
		
		if(res != curNumToRead){//if end of file reached
			break;
		}
	}
	this->curPos -= bytesRead;//make correction to curPos, since we were using Read()
	return bytesRead;
}



void File::MakeDir(){
	throw File::Exc("Make directory is not supported");
}



namespace{
const size_t DReadBlockSize = 4 * 1024;

//Define a class derived from StaticBuffer. This is just to define custom
//copy constructor which will do nothing to avoid unnecessary buffer copying when
//inserting new element to the list of chunks.
struct Chunk : public std::array<std::uint8_t, DReadBlockSize>{
	inline Chunk(){}
	inline Chunk(const Chunk&){}
};
}



std::vector<std::uint8_t> File::LoadWholeFileIntoMemory(size_t maxBytesToLoad)const{
	if(this->IsOpened()){
		throw File::IllegalStateExc("file should not be opened");
	}

	File::Guard fileGuard(*this);//make sure we close the file upon exit from the function
	
	std::list<Chunk> chunks;
	
	size_t res;
	size_t bytesRead = 0;
	
	for(;;){
		if(bytesRead == maxBytesToLoad){
			break;
		}
		
		chunks.push_back(Chunk());
		
		ASSERT(maxBytesToLoad > bytesRead)
		
		size_t numBytesToRead = maxBytesToLoad - bytesRead;
		ting::util::ClampTop(numBytesToRead, chunks.back().size());
		
		res = this->Read(ting::Buffer<std::uint8_t>(&*chunks.back().begin(), numBytesToRead));

		bytesRead += res;
		
		if(res != numBytesToRead){
			ASSERT(res < chunks.back().size())
			ASSERT(res < numBytesToRead)
			if(res == 0){
				chunks.pop_back();//pop empty chunk
			}
			break;
		}
	}
	
	ASSERT(maxBytesToLoad >= bytesRead)
	
	if(chunks.size() == 0){
		return std::move(std::vector<std::uint8_t>());
	}
	
	ASSERT(chunks.size() >= 1)
	
	std::vector<std::uint8_t> ret((chunks.size() - 1) * chunks.front().size() + res);
	
	auto p = ret.begin();
	for(; chunks.size() > 1; p += chunks.front().size()){
		memcpy(&*p, &*chunks.front().begin(), chunks.front().size());
		chunks.pop_front();
	}
	
	ASSERT(chunks.size() == 1)
	ASSERT(res <= chunks.front().size())
	memcpy(&*p, &*chunks.front().begin(), res);
	ASSERT(p + res == ret.end())
	
	return std::move(ret);
}



bool File::Exists()const{
	if(this->IsDir()){
		throw File::Exc("File::Exists(): Checking for directory existence is not supported");
	}

	if(this->IsOpened()){
		return true;
	}

	//try opening and closing the file to find out if it exists or not
	ASSERT(!this->IsOpened())
	try{
		File::Guard fileGuard(const_cast<File&>(*this), File::E_Mode::READ);
	}catch(File::Exc&){
		return false;//file opening failed, assume the file does not exist
	}
	return true;//file open succeeded => file exists
}



File::Guard::Guard(File& file, E_Mode mode) :
		f(file)
{
	if(this->f.IsOpened()){
		throw File::Exc("File::Guard::Guard(): file is already opened");
	}

	const_cast<File&>(this->f).Open(mode);
}


File::Guard::Guard(const File& file) :
		f(file)
{
	if(this->f.IsOpened()){
		throw File::Exc("File::Guard::Guard(): file is already opened");
	}

	this->f.Open();
}


File::Guard::~Guard(){
	this->f.Close();
}
