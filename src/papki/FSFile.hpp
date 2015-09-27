/* The MIT License:

Copyright (c) 2009-2014 Ivan Gagis

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
 * @file Ordinary file system File implementation
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <cstdio>
#include <memory>

#include "../debug.hpp"
#include "File.hpp"
#include "../util.hpp"



namespace ting{
namespace fs{



/**
 * @brief Native OS file system implementation of File interface.
 * Implementation of a ting::File interface for native file system of the OS.
 */
class FSFile : public File{
	mutable FILE* handle = nullptr;

protected:
	void OpenInternal(E_Mode mode)override;

	void CloseInternal()const noexcept override;

	size_t ReadInternal(ting::Buffer<std::uint8_t> buf)const override;

	size_t WriteInternal(ting::Buffer<const std::uint8_t> buf)override;

	//NOTE: use default implementation of SeekForward() because of the problems with
	//      fseek() as it can set file pointer beyond the end of file.
	
	size_t SeekBackwardInternal(size_t numBytesToSeek)const override;
	
	void RewindInternal()const override;
	
public:
	/**
	 * @brief Constructor.
	 * A root directory can be set which holds the file system subtree. The file path
	 * set by SetPath() method will refer to a file path relative to the root directory.
	 * That means that all file operations like opening the file and other will be 
	 * performed on the actual file/directory referred by the final path which is a concatenation of
	 * the root directory and the path returned by Path() method. 
     * @param pathName - initial path to set passed to File constructor.
     */
	FSFile(const std::string& pathName = std::string()) :
			File(pathName)
	{}
	
	/**
	 * @brief Destructor.
	 * This destructor calls the Close() method.
	 */
	virtual ~FSFile()noexcept{
		this->Close();
	}	
	
	bool Exists()const override;
	
	void MakeDir()override;

	/**
	 * @brief Get user home directory.
	 * Returns an absolute path to the current user's home directory.
	 * On *nix systems it will be something like "/home/user/".
     * @return Absolute path to the user's home directory.
     */
	static std::string GetHomeDir();



	virtual std::vector<std::string> ListDirContents(size_t maxEntries = 0)const override;
	
	virtual std::unique_ptr<File> Spawn()override;
	
	/**
	 * @brief Create new instance managed by auto-pointer.
     * @param pathName - path to a file.
     * @return Auto-pointer holding a new FSFile instance.
     */
	static std::unique_ptr<FSFile> New(const std::string& pathName = std::string()){
		return std::unique_ptr<FSFile>(new FSFile(pathName));
	}
};



}//~namespace
}//~namespace
