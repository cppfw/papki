/*
The MIT License (MIT)

Copyright (c) 2015-2023 Ivan Gagis <igagis@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/* ================ LICENSE END ================ */

#include "span_file.hpp"

#include <algorithm>
#include <cstring>

using namespace papki;

void span_file::open_internal(mode io_mode)
{
	if (this->is_ready_only && io_mode != mode::read) {
		throw std::logic_error("could not open span_file for writing, the file is read only");
	}
	this->ptr = this->data.begin();
}

size_t span_file::read_internal(utki::span<uint8_t> buf) const
{
	ASSERT(this->ptr <= this->data.end())
	size_t num_bytes_read = std::min(buf.size_bytes(), size_t(this->data.end() - this->ptr));
	memcpy(&*buf.begin(), &*this->ptr, num_bytes_read);
	this->ptr += num_bytes_read;
	ASSERT(this->data.overlaps(&*this->ptr) || this->ptr == this->data.end())
	return num_bytes_read;
}

size_t span_file::write_internal(utki::span<const uint8_t> buf)
{
	ASSERT(this->ptr <= this->data.end())
	size_t num_bytes_written = std::min(buf.size_bytes(), size_t(this->data.end() - this->ptr));
	memcpy(&*this->ptr, &*buf.begin(), num_bytes_written);
	this->ptr += num_bytes_written;
	ASSERT(this->data.overlaps(&*this->ptr) || this->ptr == this->data.end())
	return num_bytes_written;
}

size_t span_file::seek_forward_internal(size_t num_bytes_to_seek) const
{
	ASSERT(this->ptr <= this->data.end())
	num_bytes_to_seek = std::min(size_t(this->data.end() - this->ptr), num_bytes_to_seek);
	this->ptr += num_bytes_to_seek;
	ASSERT(this->data.overlaps(&*this->ptr) || this->ptr == this->data.end())
	return num_bytes_to_seek;
}

size_t span_file::seek_backward_internal(size_t num_bytes_to_seek) const
{
	ASSERT(this->ptr >= this->data.begin())
	num_bytes_to_seek = std::min(size_t(this->ptr - this->data.begin()), num_bytes_to_seek);
	this->ptr -= num_bytes_to_seek;
	ASSERT(this->data.overlaps(&*this->ptr) || this->ptr == this->data.end())
	return num_bytes_to_seek;
}

void span_file::rewind_internal() const
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
	this->ptr = const_cast<decltype(this->data)::value_type*>(this->data.data());
}
