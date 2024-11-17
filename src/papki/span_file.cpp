/*
The MIT License (MIT)

Copyright (c) 2015-2024 Ivan Gagis <igagis@gmail.com>

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

#include <utki/util.hpp>

using namespace papki;

void span_file::open_internal(mode io_mode)
{
	if (this->is_ready_only && io_mode != mode::read) {
		throw std::logic_error("could not open span_file for writing, the file is read only");
	}
	this->iter = this->data.begin();
}

size_t span_file::read_internal(utki::span<uint8_t> buf) const
{
	ASSERT(this->iter <= this->data.end())
	size_t num_bytes_read = std::min(buf.size(), size_t(this->data.end() - this->iter));
	auto end = utki::next(this->iter, num_bytes_read);
	std::copy(this->iter, end, buf.begin());
	this->iter = end;
	ASSERT(utki::overlaps(this->data, &*this->iter) || this->iter == this->data.end())
	return num_bytes_read;
}

size_t span_file::write_internal(utki::span<const uint8_t> buf)
{
	ASSERT(this->iter <= this->data.end())
	size_t num_bytes_written = std::min(buf.size_bytes(), size_t(this->data.end() - this->iter));
	std::copy(buf.begin(), utki::next(buf.begin(), num_bytes_written), this->iter);
	utki::next(this->iter, num_bytes_written);
	ASSERT(utki::overlaps(this->data, &*this->iter) || this->iter == this->data.end())
	return num_bytes_written;
}

size_t span_file::seek_forward_internal(size_t num_bytes_to_seek) const
{
	ASSERT(this->iter <= this->data.end())
	num_bytes_to_seek = std::min(size_t(this->data.end() - this->iter), num_bytes_to_seek);
	this->iter += num_bytes_to_seek;
	ASSERT(utki::overlaps(this->data, &*this->iter) || this->iter == this->data.end())
	return num_bytes_to_seek;
}

size_t span_file::seek_backward_internal(size_t num_bytes_to_seek) const
{
	ASSERT(this->iter >= this->data.begin())
	num_bytes_to_seek = std::min(size_t(this->iter - this->data.begin()), num_bytes_to_seek);
	this->iter -= num_bytes_to_seek;
	ASSERT(utki::overlaps(this->data, &*this->iter) || this->iter == this->data.end())
	return num_bytes_to_seek;
}

void span_file::rewind_internal() const
{
	this->iter = this->data.begin();
}
