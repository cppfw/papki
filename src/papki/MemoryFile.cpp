/* 
 * File:   MemoryFile.cpp
 * Author: igagis
 * 
 * Created on January 31, 2014, 4:17 PM
 */

#include "MemoryFile.hpp"

#include <algorithm>

using namespace ting;
using namespace fs;



void MemoryFile::OpenInternal(E_Mode mode){
	this->idx = 0;
}



size_t MemoryFile::ReadInternal(ting::Buffer<std::uint8_t> buf)const{
	ASSERT(this->idx <= this->data.size())
	size_t numBytesRead = std::min(buf.SizeInBytes(), this->data.size() - this->idx);
	memcpy(buf.begin(), &this->data[idx], numBytesRead);
	this->idx += numBytesRead;
	ASSERT(this->idx <= this->data.size())
	return numBytesRead;
}



size_t MemoryFile::WriteInternal(ting::Buffer<const std::uint8_t> buf){
	ASSERT(this->idx <= this->data.size())
	
	size_t numBytesTillEOF = this->data.size() - this->idx;
	if(numBytesTillEOF < buf.SizeInBytes()){
		size_t numBytesToGrow = buf.SizeInBytes() - numBytesTillEOF;
		this->data.resize(this->data.size() + numBytesToGrow);
	}
	
	size_t numBytesWritten = std::min(buf.SizeInBytes(), this->data.size() - this->idx);
	memcpy(&this->data[this->idx], buf.begin(), numBytesWritten);
	this->idx += numBytesWritten;
	ASSERT(this->idx <= this->data.size())
	return numBytesWritten;
}



//override
size_t MemoryFile::SeekForwardInternal(size_t numBytesToSeek)const{
	ASSERT(this->idx <= this->data.size())
	numBytesToSeek = std::min(this->data.size() - this->idx, numBytesToSeek);
	this->idx += numBytesToSeek;
	ASSERT(this->idx <= this->data.size())
	return numBytesToSeek;
}



//override
size_t MemoryFile::SeekBackwardInternal(size_t numBytesToSeek)const{
	ASSERT(this->idx <= this->data.size())
	numBytesToSeek = std::min(this->idx, numBytesToSeek);
	this->idx -= numBytesToSeek;
	ASSERT(this->idx <= this->data.size())
	return numBytesToSeek;
}



//override
void MemoryFile::RewindInternal()const{
	this->idx = 0;
}
