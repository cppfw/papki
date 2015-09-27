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
 * @file File abstract interface
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <string>
#include <memory>

#include "../debug.hpp"
#include "../types.hpp"
#include "../Buffer.hpp"

#include "../Exc.hpp"


namespace ting{
namespace fs{



/**
 * @brief Abstract interface to a file system.
 * This class represents an abstract interface to a file system.
 */
class File{
	mutable std::string path;

	mutable bool isOpened = false;
	
	mutable size_t curPos = 0;//holds current position from file beginning
	
public:
	/**
	 * @brief Basic exception class.
	 */
	class Exc : public ting::Exc{
	public:
		/**
		 * @brief Constructor.
		 * @param descr - human readable description of the error.
		 */
		Exc(const std::string& descr) :
				ting::Exc(std::string("[File::Exc]: ") + descr)
		{}
	};

	/**
	 * @brief Illegal state exception.
	 * This exception is usually thrown when trying to perform some operation on the
	 * object while the object is in inappropriate state for that operation. For example,
	 * when trying to read from file while it is not opened.
	 */
	class IllegalStateExc : public Exc{
	public:
		/**
		 * @brief Constructor.
         * @param descr - human readable description of the error.
         */
		IllegalStateExc(const std::string& descr = "Illegal opened/closed state") :
				Exc(descr)
		{}
	};
	
	/**
	 * @brief Modes of opening the file.
	 */
	enum class E_Mode{
		READ,  ///Open existing file for read only
		WRITE, ///Open existing file for read and write
		CREATE ///Create new file and open it for read and write. If file exists it will be replaced by empty file.
	};

protected:
	E_Mode ioMode;//mode only matters when file is opened

	/**
	 * @brief Constructor.
	 * @param pathName - initial path to set to the newly created File instance.
	 */
	File(const std::string& pathName = std::string()) :
			path(pathName)
	{}

public:
	File(const File&) = delete;
	File(File&&) = delete;
	File& operator=(const File&) = delete;

	/**
	 * @brief Destructor.
	 * This destructor does not call Close() method, but it has an ASSERT which checks if the file is closed.
	 * The file shall be closed upon the object destruction, all the implementations should
	 * assure that.
	 */
	virtual ~File()noexcept{
		ASSERT(!this->IsOpened())
	}

	/**
	 * @brief Set the path for this File instance.
	 * @param pathName - the path to a file or directory.
	 */
	void SetPath(const std::string& pathName)const{
		if(this->IsOpened()){
			throw File::IllegalStateExc("Cannot set path when file is opened");
		}

		this->SetPathInternal(pathName);
	}

protected:
	virtual void SetPathInternal(const std::string& pathName)const{
		this->path = pathName;
	}
	
public:
	
	/**
	 * @brief Get the current path being held by this File instance.
	 * @return The path this File instance holds.
	 */
	const std::string& Path()const noexcept{
		return this->path;
	}

	/**
	 * @brief Get current position from beginning of the file.
     * @return Current position from beginning of the file
     */
	size_t CurPos()const noexcept{
		return this->curPos;
	}

	/**
	 * @brief Get file extension.
	 * Returns a string containing the tail part of the file path, everything that
	 * goes after the last dot character ('.') in the file path string.
	 * I.e. if the file path is '/home/user/some.file.txt' then the return value
	 * will be 'txt'.
	 * Note, that on *nix systems if the file name starts with a dot then this file is treated as hidden,
	 * in that case it is thought that the file has no extension. I.e., for example
	 * , if the file path is '/home/user/.myfile' then the file has no extension and this function
	 * will return an empty string. Although, if the file path is '/home/user/.myfile.txt' then the file
	 * does have an extension and the function will return 'txt'.
	 * @return String representing file extension.
	 */
	std::string Ext()const;

	/**
	 * @brief Get directory part of the path.
	 * Example: if path is '/home/user/some.file.txt' then the return value
	 * will be '/home/user/'.
     * @return String representation of directory part of the path.
     */
	std::string Dir()const;
	
	/**
	 * @brief Get file part of the path.
	 * Example: if path is '/home/user/some.file.txt' then the return value
	 * will be 'some.file.txt'.
     * @return String representation of directory part of the path.
     */
	std::string NotDir()const;
	
	/**
	 * @brief Open file.
	 * Opens file for reading/writing or creates the file.
	 * @param mode - file opening mode (reading/writing/create).
	 * @throw IllegalStateExc - if file is already opened.
	 */
	void Open(E_Mode mode){
		if(this->IsOpened()){
			throw IllegalStateExc();
		}
		this->OpenInternal(mode);
		
		//set open mode
		if(mode == E_Mode::CREATE){
			this->ioMode = E_Mode::WRITE;
		}else{
			this->ioMode = mode;
		}

		this->isOpened = true;
		
		this->curPos = 0;
	};
	
	/**
	 * @brief Open file for reading.
	 * This is equivalent to Open(E_Mode::READ);
	 * @throw IllegalStateExc - if file is already opened.
     */
	void Open()const{
		const_cast<File*>(this)->Open(E_Mode::READ);
	}
	
protected:
	/**
	 * @brief Open file, internal implementation.
	 * Derived class should override this function with its own implementation.
     * @param mode - opening mode.
     */
	virtual void OpenInternal(E_Mode mode) = 0;
	
public:
	/**
	 * @brief Close file.
	 */
	void Close()const noexcept{
		if(!this->IsOpened()){
			return;
		}
		this->CloseInternal();
		this->isOpened = false;
	}
	
protected:
	/**
	 * @brief Close file, internal implementation.
	 * Derived class should override this function with its own implementation.
     */
	virtual void CloseInternal()const noexcept = 0;
	
public:
	/**
	 * @brief Check if the file is opened.
	 * @return true - if the file is opened.
	 * @return false - otherwise.
	 */
	bool IsOpened()const noexcept{
		return this->isOpened;
	}

	/**
	 * @brief Returns true if path points to directory.
	 * Determines if the current path is a directory.
	 * This function just checks if the path finishes with '/'
	 * character, and if it does then it is a directory.
	 * Empty path refers to the current directory.
	 * @return true - if current path points to a directory.
	 * @return false - otherwise.
	 */
	bool IsDir()const noexcept;

	/**
	 * @brief Get list of files and subdirectories of a directory.
	 * If this File instance holds a path to a directory then this method
	 * can be used to obtain the contents of the directory.
	 * @param maxEntries - maximum number of entries in the returned list. 0 means no limit.
	 * @return The array of string objects representing the directory entries.
	 */
	virtual std::vector<std::string> ListDirContents(size_t maxEntries = 0)const;

	/**
	 * @brief Read data from file.
	 * All sane file systems should support file reading.
	 * Returns number of bytes actually read. It always reads the requested number
	 * of bytes, unless end of file reached, in which case the return value will
	 * be less than number of bytes to read was requested by argument.
	 * @param buf - buffer where to store the read data.
	 * @return Number of bytes actually read. Shall always be equal to number of bytes requested to read
	 *         except the case when end of file reached.
	 * @throw IllegalStateExc - if file is not opened.
	 */
	size_t Read(ting::Buffer<std::uint8_t> buf)const;

protected:
	/**
	 * @brief Read data from file.
	 * Override this function to implement reading routine. This function is called
	 * by Read() method after it has done some safety checks.
	 * It is assumed that the whole passed buffer needs to be filled with data.
     * @param buf - buffer to fill with read data.
     * @return number of bytes actually read.
     */
	virtual size_t ReadInternal(ting::Buffer<std::uint8_t> buf)const{
		throw ting::Exc("WriteInternal(): unsupported");
	}
	
public:
	/**
	 * @brief Write data to file.
	 * Not all file systems support writing to a file, some file systems are read-only.
	 * @param buf - buffer holding the data to write.
	 * @return Number of bytes actually written. Normally, should always write all the passed data,
	 *         the only reasonable case when less data is written is when there is no more free space
	 *         in the file system.
	 * @throw IllegalStateExc - if file is not opened or opened for reading only.
	 */
	size_t Write(ting::Buffer<const std::uint8_t> buf);

protected:
	/**
	 * @brief Write data to file.
	 * Override this function to implement writing routine. This function is called
	 * by Write() method after it has done some safety checks.
	 * It is assumed that the whole passed buffer needs to be written to the file.
     * @param buf - buffer containing the data to write.
     * @return number of bytes actually written.
     */
	virtual size_t WriteInternal(ting::Buffer<const std::uint8_t> buf){
		throw ting::Exc("WriteInternal(): unsupported");
	}
	
public:
	/**
	 * @brief Seek forward.
	 * Seek file pointer forward relatively to current position.
	 * There is a default implementation of this function which uses Read() method
	 * to skip the specified number of bytes by reading the data and wasting it away.
	 * It will not go beyond the end of file.
	 * @param numBytesToSeek - number of bytes to skip.
	 * @return number of bytes actually skipped.
	 * @throw IllegalStateExc - if file is not opened.
	 */
	size_t SeekForward(size_t numBytesToSeek)const{
		if(!this->IsOpened()){
			throw File::IllegalStateExc("SeekForward(): file is not opened");
		}
		size_t ret = this->SeekForwardInternal(numBytesToSeek);
		this->curPos += ret;
		return ret;
	}
	
protected:
	/**
	 * @brief Seek forward, internal implementation.
	 * This function is called by SeekForward() after it has done some safety checks.
	 * Derived class may override this function with its own implementation.
	 * Otherwise, there is a default implementation which just reads and wastes
	 * necessary amount of bytes.
     * @param numBytesToSeek - number of bytes to seek.
	 * @return number of bytes actually skipped.
     */
	virtual size_t SeekForwardInternal(size_t numBytesToSeek)const;
	
public:

	/**
	 * @brief Seek backwards.
	 * Seek file pointer backwards relatively to he current position. Not all file systems
	 * support seeking backwards.
	 * @param numBytesToSeek - number of bytes to skip.
	 * @return number of bytes actually skipped.
	 * @throw IllegalStateExc - if file is not opened.
	 */
	size_t SeekBackward(size_t numBytesToSeek)const{
		if(!this->IsOpened()){
			throw File::IllegalStateExc("SeekForward(): file is not opened");
		}
		size_t ret = this->SeekBackwardInternal(numBytesToSeek);
		ASSERT(ret <= this->curPos)
		this->curPos -= ret;
		return ret;
	}
	
protected:
	/**
	 * @brief Seek backwards, internal implementation.
	 * This function is called by SeekBackward() after it has done some safety checks.
	 * Derived class may override this function with its own implementation.
     * @param numBytesToSeek - number of bytes to seek.
	 * @return number of bytes actually skipped.
     */
	virtual size_t SeekBackwardInternal(size_t numBytesToSeek)const{
		throw ting::Exc("SeekBackward(): unsupported");
	}
	
public:

	/**
	 * @brief Seek to the beginning of the file.
	 * There is a default implementation of this operation by just closing and opening the file again.
	 * @throw IllegalStateExc - if file is not opened.
	 */
	void Rewind()const{
		if(!this->IsOpened()){
			throw File::IllegalStateExc("Rewind(): file is not opened");
		}
		this->RewindInternal();
		this->curPos = 0;
	}
	
protected:
	/**
	 * @brief Rewind, internal implementation.
	 * This function is called by Rewind() after it has done some safety checks.
	 * Derived class may override this function with its own implementation.
     */
	virtual void RewindInternal()const{
		E_Mode m = this->ioMode;
		this->Close();
		const_cast<File*>(this)->Open(m);
	}
	
public:

	/**
	 * @brief Create directory.
	 * If this File instance is a directory then try to create that directory on
	 * file system. Not all file systems are writable, so not all of them support
	 * directory creation.
	 * @throw IllegalStateExc - if file is opened.
	 */
	virtual void MakeDir();

public:
	/**
	 * @brief Load the entire file into the RAM.
	 * @param maxBytesToLoad - maximum bytes to load. Default value is the maximum limit the size_t type can hold.
	 * @return Array containing loaded file data.
	 * @throw IllegalStateExc - if file is already opened.
	 */
	std::vector<std::uint8_t> LoadWholeFileIntoMemory(size_t maxBytesToLoad = size_t(-1))const;

	/**
	 * @brief Check for file/directory existence.
	 * @return true - if file/directory exists.
	 * @return false - otherwise.
	 */
	virtual bool Exists()const;

	
	/**
	 * @brief Creates another File object of same implementation.
	 * Creates a 'clone' of the file system implementation represented by this object.
	 * It does not copy current file path or open/close state etc. Spawned object is in
	 * initial state.
     * @return Newly spawned File object.
     */
	virtual std::unique_ptr<File> Spawn() = 0;
	
	std::unique_ptr<const File> Spawn()const{
		return const_cast<File*>(this)->Spawn();
	}
	
public:
	/**
	 * @brief File guard class.
	 * Use this class to open the file within the particular scope.
	 * As the file guard object goes out of the scope it will close the file in its destructor.
	 * Usage:
	 * @code
	 *	File& fi;//assume we have some ting::File object visible in current scope.
	 *	...
	 *	{
	 *		//assume the 'fi' is closed.
	 *		//Let's create the file guard object. This will open the file 'fi'
	 *		// for reading by calling fi.Open(ting::File::READ) method.
	 *		ting::File::Guard fileGuard(fi, ting::File::READ);
	 * 
	 *		...
	 *		//do some reading
	 *		fi.Read(...);
	 *		
	 *		//going out of scope will destroy the 'fileGuard' object. In turn,
	 *		//it will automatically close the file 'fi' in its destructor by
	 *		//calling fi.Close() method.
	 *	}
	 * @endcode
	 */
	class Guard{
		const File& f;
	public:
		Guard(File &file, E_Mode mode);

		Guard(const File &file);
		
		~Guard();
	};
};



}//~namespace
}//~namespace
