#include "vector_file.hpp"

#include <algorithm>
#include <cstring>

using namespace papki;



void vector_file::open_internal(mode mode){
	this->idx = 0;
}



size_t vector_file::read_internal(utki::span<uint8_t> buf)const{
	ASSERT(this->idx <= this->data.size())
	size_t numBytesRead = std::min(buf.size_bytes(), this->data.size() - this->idx);
	memcpy(buf.begin(), &this->data[idx], numBytesRead);
	this->idx += numBytesRead;
	ASSERT(this->idx <= this->data.size())
	return numBytesRead;
}



size_t vector_file::write_internal(utki::span<const uint8_t> buf){
	ASSERT(this->idx <= this->data.size())
	
	size_t numBytesTillEOF = this->data.size() - this->idx;
	if(numBytesTillEOF < buf.size_bytes()){
		size_t numBytesToGrow = buf.size_bytes() - numBytesTillEOF;
		this->data.resize(this->data.size() + numBytesToGrow);
	}
	
	size_t numBytesWritten = std::min(buf.size_bytes(), this->data.size() - this->idx);
	memcpy(&this->data[this->idx], buf.begin(), numBytesWritten);
	this->idx += numBytesWritten;
	ASSERT(this->idx <= this->data.size())
	return numBytesWritten;
}



size_t vector_file::seek_forward_internal(size_t numBytesToSeek)const{
	ASSERT(this->idx <= this->data.size())
	numBytesToSeek = std::min(this->data.size() - this->idx, numBytesToSeek);
	this->idx += numBytesToSeek;
	ASSERT(this->idx <= this->data.size())
	return numBytesToSeek;
}

size_t vector_file::seek_backward_internal(size_t numBytesToSeek)const{
	ASSERT(this->idx <= this->data.size())
	numBytesToSeek = std::min(this->idx, numBytesToSeek);
	this->idx -= numBytesToSeek;
	ASSERT(this->idx <= this->data.size())
	return numBytesToSeek;
}

void vector_file::rewind_internal()const{
	this->idx = 0;
}
