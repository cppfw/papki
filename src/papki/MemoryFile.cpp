#include "MemoryFile.hpp"

#include <algorithm>
#include <cstring>

using namespace papki;



void MemoryFile::open_internal(mode mode){
	this->idx = 0;
}



size_t MemoryFile::read_internal(utki::span<std::uint8_t> buf)const{
	ASSERT(this->idx <= this->data.size())
	size_t numBytesRead = std::min(buf.sizeInBytes(), this->data.size() - this->idx);
	memcpy(buf.begin(), &this->data[idx], numBytesRead);
	this->idx += numBytesRead;
	ASSERT(this->idx <= this->data.size())
	return numBytesRead;
}



size_t MemoryFile::write_internal(const utki::span<std::uint8_t> buf){
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



size_t MemoryFile::seek_forward_internal(size_t numBytesToSeek)const{
	ASSERT(this->idx <= this->data.size())
	numBytesToSeek = std::min(this->data.size() - this->idx, numBytesToSeek);
	this->idx += numBytesToSeek;
	ASSERT(this->idx <= this->data.size())
	return numBytesToSeek;
}

size_t MemoryFile::seek_backward_internal(size_t numBytesToSeek)const{
	ASSERT(this->idx <= this->data.size())
	numBytesToSeek = std::min(this->idx, numBytesToSeek);
	this->idx -= numBytesToSeek;
	ASSERT(this->idx <= this->data.size())
	return numBytesToSeek;
}

void MemoryFile::rewind_internal()const{
	this->idx = 0;
}
