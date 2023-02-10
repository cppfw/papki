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

#include "vector_file.hpp"

#include <algorithm>
#include <cstring>

using namespace papki;

void vector_file::open_internal(mode mode)
{
	this->idx = 0;
}

size_t vector_file::read_internal(utki::span<uint8_t> buf) const
{
	ASSERT(this->idx <= this->data.size())
	size_t num_bytes_read = std::min(buf.size_bytes(), this->data.size() - this->idx);

	auto i = std::next(this->data.begin(), this->idx);
	std::copy(i, std::next(i, num_bytes_read), buf.begin());

	this->idx += num_bytes_read;
	ASSERT(this->idx <= this->data.size())
	return num_bytes_read;
}

size_t vector_file::write_internal(utki::span<const uint8_t> buf)
{
	ASSERT(this->idx <= this->data.size())

	size_t num_bytes_till_eof = this->data.size() - this->idx;
	if (num_bytes_till_eof < buf.size_bytes()) {
		size_t num_bytes_to_grow = buf.size_bytes() - num_bytes_till_eof;
		this->data.resize(this->data.size() + num_bytes_to_grow);
	}

	size_t num_bytes_written = std::min(buf.size_bytes(), this->data.size() - this->idx);

	auto start_iter = this->data.begin();

	for (decltype(this->idx) num_to_seek = this->idx;;) {
		const auto max_seek = std::numeric_limits<ptrdiff_t>::max();
		if (num_to_seek > max_seek) {
			num_to_seek -= max_seek;
			std::next(start_iter, max_seek);
		} else {
			std::next(start_iter, num_to_seek);
			break;
		}
	}

	std::copy(buf.begin(), buf.end(), start_iter);

	this->idx += num_bytes_written;
	ASSERT(this->idx <= this->data.size())
	return num_bytes_written;
}

size_t vector_file::seek_forward_internal(size_t num_bytes_to_seek) const
{
	ASSERT(this->idx <= this->data.size())
	num_bytes_to_seek = std::min(this->data.size() - this->idx, num_bytes_to_seek);
	this->idx += num_bytes_to_seek;
	ASSERT(this->idx <= this->data.size())
	return num_bytes_to_seek;
}

size_t vector_file::seek_backward_internal(size_t num_bytes_to_seek) const
{
	ASSERT(this->idx <= this->data.size())
	num_bytes_to_seek = std::min(this->idx, num_bytes_to_seek);
	this->idx -= num_bytes_to_seek;
	ASSERT(this->idx <= this->data.size())
	return num_bytes_to_seek;
}

void vector_file::rewind_internal() const
{
	this->idx = 0;
}
