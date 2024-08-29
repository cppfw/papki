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

#include "file.hpp"

#include <cstring>
#include <list>

using namespace papki;

void file::open(mode io_mode)
{
	if (this->is_open()) {
		throw std::logic_error("papki::file::open(): file is already opened");
	}
	if (this->is_dir()) {
		throw std::logic_error("file refers to directory, directory cannot be opened");
	}
	this->open_internal(io_mode);

	// set open mode
	if (io_mode == mode::create) {
		this->io_mode = mode::write;
	} else {
		this->io_mode = io_mode;
	}

	this->is_file_open = true;

	this->current_pos = 0;
};

void file::close() const noexcept
{
	if (!this->is_open()) {
		return;
	}
	this->close_internal();
	this->is_file_open = false;
}

bool file::is_dir() const noexcept
{
	if (this->path().size() == 0) {
		return false;
	}

	ASSERT(this->path().size() > 0)
	if (this->path().back() == '/') {
		return true;
	}

	return false;
}

std::vector<std::string> file::list_dir(size_t max_entries) const
{
	throw std::runtime_error("file::list_dir(): not supported for this file instance");
}

size_t file::read(utki::span<uint8_t> buf) const
{
	if (!this->is_open()) {
		throw std::logic_error("Cannot read, file is not opened");
	}

	size_t ret = this->read_internal(buf);
	this->current_pos += ret;
	return ret;
}

size_t file::write(utki::span<const uint8_t> buf)
{
	if (!this->is_open()) {
		throw std::logic_error("Cannot write, file is not opened");
	}

	if (this->io_mode != mode::write) {
		throw std::logic_error("file is opened, but not in write mode");
	}

	size_t ret = this->write_internal(buf);
	this->current_pos += ret;
	return ret;
}

size_t file::seek_forward(size_t num_bytes_to_seek) const
{
	if (!this->is_open()) {
		throw std::logic_error("seek_forward(): file is not opened");
	}
	size_t ret = this->seek_forward_internal(num_bytes_to_seek);
	this->current_pos += ret;
	return ret;
}

size_t file::seek_forward_internal(size_t num_bytes_to_seek) const
{
	constexpr size_t num_bytes_in_kilobyte = 1024;
	std::array<uint8_t, 4 * num_bytes_in_kilobyte> buf{}; // 4kb buffer

	size_t num_bytes_read = 0;
	for (; num_bytes_read != num_bytes_to_seek;) {
		size_t cur_num_to_read = num_bytes_to_seek - num_bytes_read;
		cur_num_to_read = std::min(cur_num_to_read, buf.size()); // clamp top
		size_t res = this->read(utki::make_span(&*buf.begin(), cur_num_to_read));
		ASSERT(num_bytes_read < num_bytes_to_seek)
		ASSERT(num_bytes_to_seek >= res)
		ASSERT(num_bytes_read <= num_bytes_to_seek - res)
		num_bytes_read += res;

		if (res != cur_num_to_read) { // if end of file reached
			break;
		}
	}
	this->current_pos -= num_bytes_read; // make correction to curPos, since we were using read()
	return num_bytes_read;
}

size_t file::seek_backward(size_t num_bytes_to_seek) const
{
	if (!this->is_open()) {
		throw std::logic_error("seek_backward(): file is not opened");
	}
	size_t ret = this->seek_backward_internal(num_bytes_to_seek);
	ASSERT(ret <= this->current_pos)
	this->current_pos -= ret;
	return ret;
}

size_t file::seek_backward_internal(size_t num_bytes_to_seek) const
{
	throw std::runtime_error("seek_backward() is unsupported");
}

void file::make_dir()
{
	throw std::runtime_error("make_dir() is not supported");
}

void file::rewind() const
{
	if (!this->is_open()) {
		throw std::logic_error("rewind(): file is not open");
	}
	this->rewind_internal();
	this->current_pos = 0;
}

void file::rewind_internal() const
{
	mode m = this->io_mode;
	this->close();
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
	const_cast<file*>(this)->open(m);
}

std::unique_ptr<file> file::spawn(std::string path)
{
	auto ret = this->spawn();
	ret->set_path(std::move(path));
	return ret;
}

std::vector<uint8_t> file::load(size_t max_bytes_to_load) const
{
	if (this->is_open()) {
		throw std::logic_error("file::load(): file should not be open");
	}

	std::vector<uint8_t> ret;

	file::guard file_guard(*this); // make sure we close the file upon exit from the function

	const size_t read_chunk_size = 0x1000; // 4kb

	for (size_t num_bytes_read = 0; max_bytes_to_load != 0; num_bytes_read += read_chunk_size) {
		auto num_bytes_to_read = std::min(max_bytes_to_load, read_chunk_size);
		ret.resize(ret.size() + num_bytes_to_read);
		auto n = this->read(utki::make_span(&ret[num_bytes_read], num_bytes_to_read));
		ASSERT(n <= num_bytes_to_read)
		if (n != num_bytes_to_read) {
			ret.resize(num_bytes_read + n);
			return ret;
		}
		ASSERT(max_bytes_to_load >= num_bytes_to_read)
		max_bytes_to_load -= num_bytes_to_read;
	}

	return ret;
}

bool file::exists() const
{
	if (this->is_dir()) {
		// TODO: implement checking for directory existance
		throw std::logic_error(
			"file::exists(): path is a directory, checking for "
			"directory existence is not yet supported"
		);
	}

	if (this->is_open()) {
		return true;
	}

	// try opening and closing the file to find out if it exists or not
	ASSERT(!this->is_open())
	try {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
		file::guard file_guard(const_cast<file&>(*this), file::mode::read);
	} catch (std::runtime_error&) {
		return false; // file opening failed, assume the file does not exist
	}
	return true; // file open succeeded => file exists
}

uint64_t file::size() const
{
	if (this->is_open()) {
		throw std::logic_error("file must not be open when calling file::size() method");
	}

	file::guard file_guard(*this, file::mode::read);

	return this->seek_forward(~0);
}

file::guard::guard(const file& f, mode io_mode) :
	f(f)
{
	if (this->f.is_open()) {
		throw std::logic_error("file::guard::guard(): file is already opened");
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
	const_cast<file&>(this->f).open(io_mode);
}

file::guard::guard(const file& f) :
	f(f)
{
	if (this->f.is_open()) {
		throw std::logic_error("file::guard::guard(): file is already opened");
	}

	this->f.open();
}

file::guard::~guard()
{
	this->f.close();
}
