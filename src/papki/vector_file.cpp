#include "vector_file.hpp"

#include <algorithm>
#include <cstring>

using namespace papki;

void vector_file::open_internal(mode mode){
	this->idx = 0;
}

size_t vector_file::read_internal(utki::span<uint8_t> buf)const{
	ASSERT(this->idx <= this->data.size())
	size_t num_bytes_read = std::min(buf.size_bytes(), this->data.size() - this->idx);

	auto i = std::next(this->data.begin(), this->idx);
	std::copy(
			i,
			std::next(i, num_bytes_read),
			buf.begin()
		);

	this->idx += num_bytes_read;
	ASSERT(this->idx <= this->data.size())
	return num_bytes_read;
}

size_t vector_file::write_internal(utki::span<const uint8_t> buf){
	ASSERT(this->idx <= this->data.size())
	
	size_t num_bytes_till_eof = this->data.size() - this->idx;
	if(num_bytes_till_eof < buf.size_bytes()){
		size_t num_bytes_to_grow = buf.size_bytes() - num_bytes_till_eof;
		this->data.resize(this->data.size() + num_bytes_to_grow);
	}
	
	size_t num_bytes_written = std::min(buf.size_bytes(), this->data.size() - this->idx);

	std::copy(buf.begin(), buf.end(), std::next(this->data.begin(), this->idx));

	this->idx += num_bytes_written;
	ASSERT(this->idx <= this->data.size())
	return num_bytes_written;
}

size_t vector_file::seek_forward_internal(size_t num_bytes_to_seek)const{
	ASSERT(this->idx <= this->data.size())
	num_bytes_to_seek = std::min(this->data.size() - this->idx, num_bytes_to_seek);
	this->idx += num_bytes_to_seek;
	ASSERT(this->idx <= this->data.size())
	return num_bytes_to_seek;
}

size_t vector_file::seek_backward_internal(size_t num_bytes_to_seek)const{
	ASSERT(this->idx <= this->data.size())
	num_bytes_to_seek = std::min(this->idx, num_bytes_to_seek);
	this->idx -= num_bytes_to_seek;
	ASSERT(this->idx <= this->data.size())
	return num_bytes_to_seek;
}

void vector_file::rewind_internal()const{
	this->idx = 0;
}
