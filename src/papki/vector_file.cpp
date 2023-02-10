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

// TODO: move this to utki
template <typename iterator_type>
iterator_type the_next(iterator_type iter, size_t num)
{
	const auto max_advance = std::numeric_limits<typename std::iterator_traits<iterator_type>::difference_type>::max();
	for (size_t num_left = num;;) {
		if (num_left > size_t(max_advance)) {
			num_left -= max_advance;
			iter = std::next(iter, max_advance);
		} else {
			return std::next(iter, num_left);
		}
	}
}

void vector_file::open_internal(mode mode)
{
	this->idx = 0;
}

size_t vector_file::read_internal(utki::span<uint8_t> buf) const
{
	ASSERT(this->idx <= this->data.size())
	size_t num_bytes_read = std::min(buf.size_bytes(), this->data.size() - this->idx);

	auto i = the_next(this->data.begin(), this->idx);
	std::copy(i, the_next(i, num_bytes_read), buf.begin());

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

	std::copy(buf.begin(), buf.end(), the_next(this->data.begin(), this->idx));

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
