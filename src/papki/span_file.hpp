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

#pragma once

#include <utki/config.hpp>

#include "file.hpp"

namespace papki {

/**
 * @brief Memory buffer file.
 * A memory buffer represented as a file.
 * It supports reading, writing, seeking forward and backwards, rewinding.
 * The size of the file remains constant and is equal to the size of the memory
 * buffer used for storing the file data.
 */
class span_file : public file
{
private:
	span_file(const span_file&) = delete;
	span_file(span_file&&) = delete;
	span_file& operator=(const span_file&) = delete;
	span_file& operator=(span_file&&) = delete;

private:
	bool is_ready_only = false;

	utki::span<uint8_t> data;
	mutable decltype(data)::iterator ptr;

public:
	/**
	 * @brief Constructor.
	 * @param data - reference to a memory buffer holding data of the file.
	 *               The reference to the buffer is saved in the span_file object,
	 *               but ownership of the buffer is not taken. Thus, the buffer
	 * should remain alive during lifetime of this span_file object.
	 */
	span_file(utki::span<uint8_t> data) :
		data(data)
	{}

	span_file(utki::span<const uint8_t> data) :
		is_ready_only(true),
		data(const_cast<uint8_t*>(data.data()), data.size())
	{}

	span_file(utki::span<const char> data) :
		span_file(utki::make_span(reinterpret_cast<const uint8_t*>(data.data()), data.size()))
	{}

	~span_file() noexcept override {}

	virtual std::unique_ptr<file> spawn() override
	{
		throw std::runtime_error("span_file::spawn(): spawning is not supported.");
	}

protected:
	void open_internal(mode io_mode) override;

	void close_internal() const noexcept override {}

	size_t read_internal(utki::span<uint8_t> buf) const override;

	size_t write_internal(utki::span<const uint8_t> buf) override;

	size_t seek_forward_internal(size_t num_bytes_to_seek) const override;

	size_t seek_backward_internal(size_t num_bytes_to_seek) const override;

	void rewind_internal() const override;
};

} // namespace papki
