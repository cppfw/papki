#include "span_file.hpp"

#include <algorithm>
#include <cstring>


using namespace papki;



void span_file::open_internal(mode mode){
	this->ptr = this->data.begin();
}



size_t span_file::read_internal(utki::span<uint8_t> buf)const {
	ASSERT(this->ptr <= this->data.end())
	size_t numBytesRead = std::min(buf.sizeInBytes(), size_t(this->data.end() - this->ptr));
	memcpy(&*buf.begin(), &*this->ptr, numBytesRead);
	this->ptr += numBytesRead;
	ASSERT(this->data.overlaps(&*this->ptr) || this->ptr == this->data.end())
	return numBytesRead;
}



size_t span_file::write_internal(const utki::span<uint8_t> buf){
	ASSERT(this->ptr <= this->data.end())
	size_t numBytesWritten = std::min(buf.sizeInBytes(), size_t(this->data.end() - this->ptr));
	memcpy(&*this->ptr, &*buf.begin(), numBytesWritten);
	this->ptr += numBytesWritten;
	ASSERT(this->data.overlaps(&*this->ptr) || this->ptr == this->data.end())
	return numBytesWritten;
}



size_t span_file::seek_forward_internal(size_t numBytesToSeek)const{
	ASSERT(this->ptr <= this->data.end())
	numBytesToSeek = std::min(size_t(this->data.end() - this->ptr), numBytesToSeek);
	this->ptr += numBytesToSeek;
	ASSERT(this->data.overlaps(&*this->ptr) || this->ptr == this->data.end())
	return numBytesToSeek;
}



size_t span_file::seek_backward_internal(size_t numBytesToSeek)const{
	ASSERT(this->ptr >= this->data.begin())
	numBytesToSeek = std::min(size_t(this->ptr - this->data.begin()), numBytesToSeek);
	this->ptr -= numBytesToSeek;
	ASSERT(this->data.overlaps(&*this->ptr) || this->ptr == this->data.end())
	return numBytesToSeek;
}



void span_file::rewind_internal()const{
	this->ptr = const_cast<decltype(this->data)::value_type*>(&*this->data.begin());
}
