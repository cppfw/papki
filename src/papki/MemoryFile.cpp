#include "MemoryFile.hpp"

#include <algorithm>
#include <cstring>

using namespace papki;



void MemoryFile::openInternal(mode mode){
	this->idx = 0;
}



size_t MemoryFile::readInternal(utki::span<std::uint8_t> buf)const{
	ASSERT(this->idx <= this->data.size())
	size_t numBytesRead = std::min(buf.sizeInBytes(), this->data.size() - this->idx);
	memcpy(buf.begin(), &this->data[idx], numBytesRead);
	this->idx += numBytesRead;
	ASSERT(this->idx <= this->data.size())
	return numBytesRead;
}



size_t MemoryFile::writeInternal(const utki::span<std::uint8_t> buf){
	ASSERT(this->idx <= this->data.size())
	
	size_t numBytesTillEOF = this->data.size() - this->idx;
	if(numBytesTillEOF < buf.sizeInBytes()){
		size_t numBytesToGrow = buf.sizeInBytes() - numBytesTillEOF;
		this->data.resize(this->data.size() + numBytesToGrow);
	}
	
	size_t numBytesWritten = std::min(buf.sizeInBytes(), this->data.size() - this->idx);
	memcpy(&this->data[this->idx], buf.begin(), numBytesWritten);
	this->idx += numBytesWritten;
	ASSERT(this->idx <= this->data.size())
	return numBytesWritten;
}



//override
size_t MemoryFile::seekForwardInternal(size_t numBytesToSeek)const{
	ASSERT(this->idx <= this->data.size())
	numBytesToSeek = std::min(this->data.size() - this->idx, numBytesToSeek);
	this->idx += numBytesToSeek;
	ASSERT(this->idx <= this->data.size())
	return numBytesToSeek;
}



//override
size_t MemoryFile::seekBackwardInternal(size_t numBytesToSeek)const{
	ASSERT(this->idx <= this->data.size())
	numBytesToSeek = std::min(this->idx, numBytesToSeek);
	this->idx -= numBytesToSeek;
	ASSERT(this->idx <= this->data.size())
	return numBytesToSeek;
}



//override
void MemoryFile::rewindInternal()const{
	this->idx = 0;
}
