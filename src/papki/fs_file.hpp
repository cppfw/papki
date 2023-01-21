/*
The MIT License (MIT)

Copyright (c) 2015-2021 Ivan Gagis <igagis@gmail.com>

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

#include <cstdio>
#include <memory>

#include <utki/debug.hpp>
#include <utki/config.hpp>

#include "file.hpp"

#ifdef assert
#	undef assert
#endif

namespace papki{

/**
 * @brief Native OS file system implementation of file interface.
 * Implementation of a papki::file interface for native file system of the OS.
 */
class fs_file : public file{
	mutable FILE* handle = nullptr;

protected:
	void open_internal(mode io_mode)override;

	void close_internal()const noexcept override;

	size_t read_internal(utki::span<uint8_t> buf)const override;

	size_t write_internal(utki::span<const uint8_t> buf)override;

	// NOTE: use default implementation of seek_forward() because of the problems with
	//       fseek(), as it can set file pointer beyond the end of file.
	
	size_t seek_backward_internal(size_t num_bytes_to_seek)const override;
	
	void rewind_internal()const override;
	
public:
	/**
	 * @brief Constructor.
	 * A root directory can be set which holds the file system subtree. The file path
	 * set by set_path() method will refer to a file path relative to the root directory.
	 * That means that all file operations like opening the file and other will be 
	 * performed on the actual file/directory referred by the final path which is a concatenation of
	 * the root directory and the path returned by path() method. 
     * @param path_name - initial path to set passed to file constructor.
     */
	fs_file(std::string_view path_name = std::string_view()) :
			file(path_name)
	{}
	
	/**
	 * @brief Destructor.
	 * This destructor calls the close() method.
	 */
	virtual ~fs_file()noexcept{
		this->close();
	}	
	
	bool exists()const override;
	
	uint64_t size()const override;

	void make_dir()override;

	/**
	 * @brief Get user home directory.
	 * Returns an absolute path to the current user's home directory.
	 * On *nix systems it will be something like "/home/user/".
     * @return Absolute path to the user's home directory.
     */
	static std::string get_home_dir();

	virtual std::vector<std::string> list_dir(size_t max_entries = 0)const override;
	
	virtual std::unique_ptr<file> spawn()override;
};

}
