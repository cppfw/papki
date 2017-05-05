/**
 * @file File abstract interface
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <string>
#include <memory>

#include <utki/debug.hpp>
#include <utki/config.hpp>
#include <utki/Buf.hpp>
#include <utki/Exc.hpp>
#include <utki/Unique.hpp>


#include "Exc.hpp"


namespace papki{



/**
 * @brief Abstract interface to a file system.
 * This class represents an abstract interface to a file system.
 */
class File : public utki::Unique{
	mutable std::string path_var;

	mutable bool isOpened_var = false;
	
	mutable size_t curPos_var = 0;//holds current position from file beginning
	
public:
	
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
			path_var(pathName)
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
		ASSERT(!this->isOpened())
	}

	/**
	 * @brief Set the path for this File instance.
	 * @param pathName - the path to a file or directory.
	 */
	void setPath(const std::string& pathName)const{
		if(this->isOpened()){
			throw papki::IllegalStateExc("Cannot set path when file is opened");
		}

		this->setPathInternal(pathName);
	}

protected:
	virtual void setPathInternal(const std::string& pathName)const{
		this->path_var = pathName;
	}
	
public:
	
	/**
	 * @brief Get the current path being held by this File instance.
	 * @return The path this File instance holds.
	 */
	const std::string& path()const noexcept{
		return this->path_var;
	}

	/**
	 * @brief Get current position from beginning of the file.
     * @return Current position from beginning of the file
     */
	size_t curPos()const noexcept{
		return this->curPos_var;
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
	std::string ext()const;

	/**
	 * @brief Get directory part of the path.
	 * Example: if path is '/home/user/some.file.txt' then the return value
	 * will be '/home/user/'.
     * @return String representation of directory part of the path.
     */
	std::string dir()const;
	
	/**
	 * @brief Get file part of the path.
	 * Example: if path is '/home/user/some.file.txt' then the return value
	 * will be 'some.file.txt'.
     * @return String representation of directory part of the path.
     */
	std::string notDir()const;
	
	/**
	 * @brief Open file.
	 * Opens file for reading/writing or creates the file.
	 * @param mode - file opening mode (reading/writing/create).
	 * @throw IllegalStateExc - if file is already opened.
	 */
	void open(E_Mode mode){
		if(this->isOpened()){
			throw IllegalStateExc();
		}
		if(this->isDir()){
			throw IllegalStateExc("File refers to directory. Directory cannot be opened.");
		}
		this->openInternal(mode);
		
		//set open mode
		if(mode == E_Mode::CREATE){
			this->ioMode = E_Mode::WRITE;
		}else{
			this->ioMode = mode;
		}

		this->isOpened_var = true;
		
		this->curPos_var = 0;
	};
	
	/**
	 * @brief Open file for reading.
	 * This is equivalent to Open(E_Mode::READ);
	 * @throw IllegalStateExc - if file is already opened.
     */
	void open()const{
		const_cast<File*>(this)->open(E_Mode::READ);
	}
	
protected:
	/**
	 * @brief Open file, internal implementation.
	 * Derived class should override this function with its own implementation.
     * @param mode - opening mode.
     */
	virtual void openInternal(E_Mode mode) = 0;
	
public:
	/**
	 * @brief Close file.
	 */
	void close()const noexcept{
		if(!this->isOpened()){
			return;
		}
		this->closeInternal();
		this->isOpened_var = false;
	}
	
protected:
	/**
	 * @brief Close file, internal implementation.
	 * Derived class should override this function with its own implementation.
     */
	virtual void closeInternal()const noexcept = 0;
	
public:
	/**
	 * @brief Check if the file is opened.
	 * @return true - if the file is opened.
	 * @return false - otherwise.
	 */
	bool isOpened()const noexcept{
		return this->isOpened_var;
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
	bool isDir()const noexcept;

	/**
	 * @brief Get list of files and subdirectories of a directory.
	 * If this File instance holds a path to a directory then this method
	 * can be used to obtain the contents of the directory.
	 * @param maxEntries - maximum number of entries in the returned list. 0 means no limit.
	 * @return The array of string objects representing the directory entries.
	 */
	virtual std::vector<std::string> listDirContents(size_t maxEntries = 0)const;

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
	size_t read(utki::Buf<std::uint8_t> buf)const;

protected:
	/**
	 * @brief Read data from file.
	 * Override this function to implement reading routine. This function is called
	 * by Read() method after it has done some safety checks.
	 * It is assumed that the whole passed buffer needs to be filled with data.
     * @param buf - buffer to fill with read data.
     * @return number of bytes actually read.
     */
	virtual size_t readInternal(utki::Buf<std::uint8_t> buf)const{
		throw utki::Exc("readInternal(): unsupported");
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
	size_t write(const utki::Buf<std::uint8_t> buf);

protected:
	/**
	 * @brief Write data to file.
	 * Override this function to implement writing routine. This function is called
	 * by Write() method after it has done some safety checks.
	 * It is assumed that the whole passed buffer needs to be written to the file.
     * @param buf - buffer containing the data to write.
     * @return number of bytes actually written.
     */
	virtual size_t writeInternal(const utki::Buf<std::uint8_t> buf){
		throw utki::Exc("writeInternal(): unsupported");
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
	size_t seekForward(size_t numBytesToSeek)const{
		if(!this->isOpened()){
			throw papki::IllegalStateExc("seekForward(): file is not opened");
		}
		size_t ret = this->seekForwardInternal(numBytesToSeek);
		this->curPos_var += ret;
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
	virtual size_t seekForwardInternal(size_t numBytesToSeek)const;
	
public:

	/**
	 * @brief Seek backwards.
	 * Seek file pointer backwards relatively to he current position. Not all file systems
	 * support seeking backwards.
	 * @param numBytesToSeek - number of bytes to skip.
	 * @return number of bytes actually skipped.
	 * @throw IllegalStateExc - if file is not opened.
	 */
	size_t seekBackward(size_t numBytesToSeek)const{
		if(!this->isOpened()){
			throw papki::IllegalStateExc("seekBackward(): file is not opened");
		}
		size_t ret = this->seekBackwardInternal(numBytesToSeek);
		ASSERT(ret <= this->curPos_var)
		this->curPos_var -= ret;
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
	virtual size_t seekBackwardInternal(size_t numBytesToSeek)const{
		throw utki::Exc("SeekBackward(): unsupported");
	}
	
public:

	/**
	 * @brief Seek to the beginning of the file.
	 * There is a default implementation of this operation by just closing and opening the file again.
	 * @throw IllegalStateExc - if file is not opened.
	 */
	void rewind()const{
		if(!this->isOpened()){
			throw papki::IllegalStateExc("Rewind(): file is not opened");
		}
		this->rewindInternal();
		this->curPos_var = 0;
	}
	
protected:
	/**
	 * @brief Rewind, internal implementation.
	 * This function is called by Rewind() after it has done some safety checks.
	 * Derived class may override this function with its own implementation.
     */
	virtual void rewindInternal()const{
		E_Mode m = this->ioMode;
		this->close();
		const_cast<File*>(this)->open(m);
	}
	
public:

	/**
	 * @brief Create directory.
	 * If this File instance is a directory then try to create that directory on
	 * file system. Not all file systems are writable, so not all of them support
	 * directory creation.
	 * @throw IllegalStateExc - if file is opened.
	 */
	virtual void makeDir();

public:
	/**
	 * @brief Load the entire file into the RAM.
	 * @param maxBytesToLoad - maximum bytes to load. Default value is the maximum limit the size_t type can hold.
	 * @return Array containing loaded file data.
	 * @throw IllegalStateExc - if file is already opened.
	 */
	std::vector<std::uint8_t> loadWholeFileIntoMemory(size_t maxBytesToLoad = size_t(-1))const;

	/**
	 * @brief Check for file/directory existence.
	 * @return true - if file/directory exists.
	 * @return false - otherwise.
	 */
	virtual bool exists()const;

	
	/**
	 * @brief Creates another File object of same implementation.
	 * Creates a 'clone' of the file system implementation represented by this object.
	 * It does not copy current file path or open/close state etc. Spawned object is in
	 * initial state.
     * @return Newly spawned File object.
     */
	virtual std::unique_ptr<File> spawn() = 0;
	
	std::unique_ptr<const File> spawn()const{
		return const_cast<File*>(this)->spawn();
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
