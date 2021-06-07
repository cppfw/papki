#include "span_file.hpp"

#include <algorithm>
#include <cstring>

using namespace papki;

void span_file::open_internal(mode io_mode){
	if(this->is_ready_only && io_mode != mode::read){
		throw std::logic_error("could not open span_file for writing, the file is read only");
	}
	this->ptr = this->data.begin();
}

size_t span_file::read_internal(utki::span<uint8_t> buf)const {
	ASSERT(this->ptr <= this->data.end())
	size_t numBytesRead = std::min(buf.size_bytes(), size_t(this->data.end() - this->ptr));
	memcpy(&*buf.begin(), &*this->ptr, numBytesRead);
	this->ptr += numBytesRead;
	ASSERT(this->data.overlaps(&*this->ptr) || this->ptr == this->data.end())
	return numBytesRead;
}

size_t span_file::write_internal(utki::span<const uint8_t> buf){
	ASSERT(this->ptr <= this->data.end())
	size_t numBytesWritten = std::min(buf.size_bytes(), size_t(this->data.end() - this->ptr));
	memcpy(&*this->ptr, &*buf.begin(), numBytesWritten);
	this->ptr += numBytesWritten;
	ASSERT(this->data.overlaps(&*this->ptr) || this->ptr == this->data.end())
	return numBytesWritten;
}

size_t span_file::seek_forward_internal(size_t num_bytes_to_seek)const{
	ASSERT(this->ptr <= this->data.end())
	num_bytes_to_seek = std::min(size_t(this->data.end() - this->ptr), num_bytes_to_seek);
	this->ptr += num_bytes_to_seek;
	ASSERT(this->data.overlaps(&*this->ptr) || this->ptr == this->data.end())
	return num_bytes_to_seek;
}

size_t span_file::seek_backward_internal(size_t num_bytes_to_seek)const{
	ASSERT(this->ptr >= this->data.begin())
	num_bytes_to_seek = std::min(size_t(this->ptr - this->data.begin()), num_bytes_to_seek);
	this->ptr -= num_bytes_to_seek;
	ASSERT(this->data.overlaps(&*this->ptr) || this->ptr == this->data.end())
	return num_bytes_to_seek;
}

void span_file::rewind_internal()const{
	this->ptr = const_cast<decltype(this->data)::value_type*>(&*this->data.begin());
}
