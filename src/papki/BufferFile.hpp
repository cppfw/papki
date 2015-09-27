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


namespace papki{

 
/**
 * @brief Memory buffer file.
 * A memory buffer represented as a File.
 * It supports reading, writing, seeking forward and backwards, rewinding.
 * The size of the file remains constant and is equal to the size of the memory
 * buffer used for storing the file data.
 */
class BufferFile : public File{
	
private:
	BufferFile(const BufferFile&) = delete;
	BufferFile(BufferFile&&) = delete;
	BufferFile& operator=(const BufferFile&) = delete;
	BufferFile& operator=(BufferFile&&) = delete;
	
private:
	utki::Buf<std::uint8_t> data;
	mutable decltype(data)::iterator ptr;
	
public:
	/**
	 * @brief Constructor.
	 * @param data - reference to a memory buffer holding data of the file.
	 *               The reference to the buffer is saved in the BufferFile object,
	 *               but ownership of the buffer is not taken. Thus, the buffer should remain alive during lifetime of this BufferFile object.
	 */
	//NOTE: ownership of the buffer is not taken, buffer must remain alive during this object's lifetime.
	BufferFile(utki::Buf<std::uint8_t> data) :
			data(data)
	{}
	
	~BufferFile()noexcept override{}

	virtual std::unique_ptr<File> spawn()override{
		throw papki::Exc("BufferFile::Spawn(): spawning is not supported.");
	}

protected:

	void OpenInternal(E_Mode mode)override;
	
	void CloseInternal()const noexcept override{}
	
	size_t readInternal(utki::Buf<std::uint8_t> buf)const override;

	size_t writeInternal(utki::Buf<const std::uint8_t> buf)override;
	
	size_t seekForwardInternal(size_t numBytesToSeek)const override;
	
	size_t seekBackwardInternal(size_t numBytesToSeek)const override;
	
	void rewindInternal()const override;
};

}//~namespace
