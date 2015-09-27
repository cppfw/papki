/* 
 * File:   BufferFile.cpp
 * Author: igagis
 * 
 * Created on January 31, 2014, 4:17 PM
 */

#include "BufferFile.hpp"

#include <algorithm>


using namespace ting;
using namespace fs;



//override
void BufferFile::OpenInternal(E_Mode mode){
	this->ptr = this->data.begin();
}



//override
size_t BufferFile::ReadInternal(ting::Buffer<std::uint8_t> buf)const {
	ASSERT(this->ptr <= this->data.end())
	size_t numBytesRead = std::min(buf.SizeInBytes(), size_t(this->data.end() - this->ptr));
	memcpy(&*buf.begin(), &*this->ptr, numBytesRead);
	this->ptr += numBytesRead;
	ASSERT(this->data.Overlaps(&*this->ptr) || this->ptr == this->data.end())
	return numBytesRead;
}



//override
size_t BufferFile::WriteInternal(ting::Buffer<const std::uint8_t> buf){
	ASSERT(this->ptr <= this->data.end())
	size_t numBytesWritten = std::min(buf.SizeInBytes(), size_t(this->data.end() - this->ptr));
	memcpy(&*this->ptr, &*buf.begin(), numBytesWritten);
	this->ptr += numBytesWritten;
	ASSERT(this->data.Overlaps(&*this->ptr) || this->ptr == this->data.end())
	return numBytesWritten;
}



//override
size_t BufferFile::SeekForwardInternal(size_t numBytesToSeek)const{
	ASSERT(this->ptr <= this->data.end())
	numBytesToSeek = std::min(size_t(this->data.end() - this->ptr), numBytesToSeek);
	this->ptr += numBytesToSeek;
	ASSERT(this->data.Overlaps(&*this->ptr) || this->ptr == this->data.end())
	return numBytesToSeek;
}



//override
size_t BufferFile::SeekBackwardInternal(size_t numBytesToSeek)const{
	ASSERT(this->ptr >= this->data.begin())
	numBytesToSeek = std::min(size_t(this->ptr - this->data.begin()), numBytesToSeek);
	this->ptr -= numBytesToSeek;
	ASSERT(this->data.Overlaps(&*this->ptr) || this->ptr == this->data.end())
	return numBytesToSeek;
}



//override
void BufferFile::RewindInternal()const{
	this->ptr = const_cast<decltype(this->data)::value_type*>(&*this->data.begin());
}
