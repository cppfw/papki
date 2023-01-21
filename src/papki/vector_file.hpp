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

#include <vector>

namespace papki{

/**
 * @brief Memory file.
 * Class representing a file stored in memory. Supports reading, writing, seeking backwards and forward, rewinding.
 */
class vector_file : public file{
	
	vector_file(const vector_file&) = delete;
	vector_file(vector_file&&) = delete;
	vector_file& operator=(const vector_file&) = delete;
	vector_file& operator=(vector_file&&) = delete;
	
private:
	std::vector<uint8_t> data;
	mutable size_t idx; // current file position
	
public:
	/**
	 * @brief Constructor.
	 * Creates empty memory file.
     */
	vector_file(){}
	
	virtual ~vector_file()noexcept{}

	/**
	 * @brief Current file size.
     * @return current size of the file.
     */
	uint64_t size()const override{
		return this->data.size();
	}
	
	virtual std::unique_ptr<file> spawn()override{
		return std::make_unique<vector_file>();
	}
	
	/**
	 * @brief Clear the data of the file.
	 * After this operation the file becomes empty.
     * @return Data previously held by this file.
     */
	decltype(data) reset_data(){
		if(this->is_open()){
			throw std::logic_error("vector_file::reset_data(): could not reset data while file is opened");
		}
		return std::move(this->data);
	}
	
protected:
	void open_internal(mode io_mode)override;
	
	void close_internal()const noexcept override{}
	
	size_t read_internal(utki::span<uint8_t> buf)const override;
	
	size_t write_internal(utki::span<const uint8_t> buf)override;
	
	size_t seek_forward_internal(size_t numBytesToSeek)const override;
	
	size_t seek_backward_internal(size_t numBytesToSeek)const override;
	
	void rewind_internal()const override;
};

}
