#include <list>
#include <cstring>

#include "file.hpp"



using namespace papki;



std::string file::suffix()const{
	size_t dotPos = this->path().rfind('.');
	if(dotPos == std::string::npos || dotPos == 0){//NOTE: dotPos is 0 for hidden files in *nix systems
		return std::string();
	}else{
		ASSERT(dotPos > 0)
		ASSERT(this->path().size() > 0)
		ASSERT(this->path().size() >= dotPos + 1)
		
		//Check for hidden file on *nix systems
		if(this->path()[dotPos - 1] == '/'){
			return std::string();
		}
		
		return std::string(this->path(), dotPos + 1, this->path().size() - (dotPos + 1));
	}
	ASSERT(false)
}



std::string file::dir()const{
	size_t slashPos = this->path().rfind('/');
	if(slashPos == std::string::npos){//no slash found
		return std::string();
	}

	ASSERT(slashPos > 0)
	ASSERT(this->path().size() > 0)
	ASSERT(this->path().size() >= slashPos + 1)

	return std::string(this->path(), 0, slashPos + 1);
}



std::string file::not_dir()const{
	size_t slashPos = this->path().rfind('/');
	if(slashPos == std::string::npos){//no slash found
		return this->path();
	}

	ASSERT(slashPos > 0)
	ASSERT(this->path().size() > 0)
	ASSERT(this->path().size() >= slashPos + 1)

	return std::string(this->path(), slashPos + 1);
}



bool file::is_dir()const noexcept{
	if(this->path().size() == 0){
		return false;
	}

	ASSERT(this->path().size() > 0)
	if(this->path()[this->path().size() - 1] == '/'){
		return true;
	}

	return false;
}



std::vector<std::string> file::list_dir(size_t maxEntries)const{
	throw std::runtime_error("file::list_dir(): not supported for this file instance");
}



size_t file::read(utki::span<std::uint8_t> buf)const{
	if(!this->isOpened()){
		throw utki::invalid_state("Cannot read, file is not opened");
	}
	
	size_t ret = this->readInternal(buf);
	this->curPos_var += ret;
	return ret;
}



size_t file::write(const utki::span<std::uint8_t> buf){
	if(!this->isOpened()){
		throw utki::invalid_state("Cannot write, file is not opened");
	}

	if(this->ioMode != mode::write){
		throw utki::invalid_state("file is opened, but not in write mode");
	}
	
	size_t ret = this->writeInternal(buf);
	this->curPos_var += ret;
	return ret;
}



size_t file::seekForwardInternal(size_t numBytesToSeek)const{
	std::array<std::uint8_t, 0x1000> buf;//4kb buffer
	
	size_t bytesRead = 0;
	for(; bytesRead != numBytesToSeek;){
		size_t curNumToRead = numBytesToSeek - bytesRead;
		utki::clampTop(curNumToRead, buf.size());
		size_t res = this->read(utki::make_span(&*buf.begin(), curNumToRead));
		ASSERT(bytesRead < numBytesToSeek)
		ASSERT(numBytesToSeek >= res)
		ASSERT(bytesRead <= numBytesToSeek - res)
		bytesRead += res;
		
		if(res != curNumToRead){//if end of file reached
			break;
		}
	}
	this->curPos_var -= bytesRead;//make correction to curPos, since we were using Read()
	return bytesRead;
}



void file::makeDir(){
	throw std::runtime_error("Make directory is not supported");
}



namespace{
const size_t DReadBlockSize = 4 * 1024;

//Define a class derived from std::array. This is just to define custom
//copy constructor which will do nothing to avoid unnecessary buffer copying when
//inserting new element to the list of chunks.
struct Chunk : public std::array<std::uint8_t, DReadBlockSize>{
	inline Chunk(){}
	inline Chunk(const Chunk&){}
};
}



std::vector<std::uint8_t> file::loadWholeFileIntoMemory(size_t maxBytesToLoad)const{
	if(this->isOpened()){
		throw utki::invalid_state("file should not be opened");
	}

	file::guard fileGuard(*this);//make sure we close the file upon exit from the function
	
	std::list<Chunk> chunks;
	
	size_t res = 0;
	size_t bytesRead = 0;
	
	for(;;){
		if(bytesRead == maxBytesToLoad){
			break;
		}
		
		chunks.push_back(Chunk());
		
		ASSERT(maxBytesToLoad > bytesRead)
		
		size_t numBytesToRead = maxBytesToLoad - bytesRead;
		utki::clampTop(numBytesToRead, chunks.back().size());
		
		res = this->read(utki::make_span(&*chunks.back().begin(), numBytesToRead));

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
		return std::vector<std::uint8_t>();
	}
	
	ASSERT(chunks.size() >= 1)
	
	std::vector<std::uint8_t> ret((chunks.size() - (res == 0 ? 0 : 1)) * chunks.front().size() + res);
	
	auto p = ret.begin();
	for(; chunks.size() > (res == 0 ? 0 : 1); p += chunks.front().size()){
		memcpy(&*p, &*chunks.front().begin(), chunks.front().size());
		chunks.pop_front();
	}
	
	ASSERT(chunks.size() == (res == 0 ? 0 : 1))
	ASSERT(res <= chunks.front().size())
	memcpy(&*p, &*chunks.front().begin(), res);
	ASSERT(p + res == ret.end())
	
	return ret;
}



bool file::exists()const{
	if(this->isDir()){
		// TODO: implement checking for directory existance
		throw utki::invalid_state("file::exists(): path is a directory, checking for directory existence is not yet supported");
	}

	if(this->isOpened()){
		return true;
	}

	// try opening and closing the file to find out if it exists or not
	ASSERT(!this->isOpened())
	try{
		file::guard fileGuard(const_cast<file&>(*this), file::mode::read);
	}catch(std::runtime_error&){
		return false; // file opening failed, assume the file does not exist
	}
	return true; // file open succeeded => file exists
}



file::guard::guard(const file& f, mode io_mode) :
		f(f)
{
	if(this->f.isOpened()){
		throw utki::invalid_state("file::guard::guard(): file is already opened");
	}

	const_cast<file&>(this->f).open(io_mode);
}


file::guard::guard(const file& f) :
		f(f)
{
	if(this->f.isOpened()){
		throw utki::invalid_state("file::guard::guard(): file is already opened");
	}

	this->f.open();
}


file::guard::~guard(){
	this->f.close();
}
