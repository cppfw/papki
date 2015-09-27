/* The MIT License:

Copyright (c) 2014 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

// Home page: http://ting.googlecode.com

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include "File.hpp"

#include "../util.hpp"

#include <vector>



namespace ting{
namespace fs{

/**
 * @brief Memory file.
 * Class representing a file stored in memory. Supports reading, writing, seeking backwards and forward, rewinding.
 */
class MemoryFile : public File{
	
	MemoryFile(const MemoryFile&) = delete;
	MemoryFile(MemoryFile&&) = delete;
	MemoryFile& operator=(const MemoryFile&) = delete;
	MemoryFile& operator=(MemoryFile&&) = delete;
	
private:
	std::vector<std::uint8_t> data;
	mutable size_t idx;
	
public:
	/**
	 * @brief Constructor.
	 * Creates empty memory file.
     */
	MemoryFile(){}
	
	virtual ~MemoryFile()noexcept{}

	/**
	 * @brief Current file size.
     * @return current size of the file.
     */
	size_t Size(){
		return this->data.size();
	}
	

	virtual std::unique_ptr<File> Spawn()override{
		return std::unique_ptr<File>(new MemoryFile);
	}

	
	/**
	 * @brief Clear the data of the file.
	 * After this operation the file becomes empty.
     * @return Data previously held by this file.
     */
	decltype(data) ResetData(){
		if(this->IsOpened()){
			throw IllegalStateExc("MemoryFile::ResetData(): could not reset data while file is opened");
		}
		return std::move(this->data);
	}
	
protected:
	void OpenInternal(E_Mode mode)override;
	
	void CloseInternal()const noexcept override{}
	
	size_t ReadInternal(ting::Buffer<std::uint8_t> buf)const override;
	
	size_t WriteInternal(ting::Buffer<const std::uint8_t> buf)override;
	
	size_t SeekForwardInternal(size_t numBytesToSeek)const override;
	
	size_t SeekBackwardInternal(size_t numBytesToSeek)const override;
	
	void RewindInternal()const override;
};

}}//~namespace
