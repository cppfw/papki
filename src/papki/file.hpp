#pragma once

#include <string>
#include <memory>
#include <stdexcept>

#include <utki/debug.hpp>
#include <utki/config.hpp>
#include <utki/span.hpp>
#include <utki/exception.hpp>

namespace papki{

/**
 * @brief Abstract interface to a file system.
 * This class represents an abstract interface to a file system.
 */
class file{
	mutable std::string path_var;

	mutable bool isOpened_var = false;
	
	mutable size_t curPos_var = 0;//holds current position from file beginning
	
public:
	
	/**
	 * @brief Modes of opening the file.
	 */
	enum class mode{
		read, /// Open existing file for read only.
		READ = read, //TODO: deprecaed, remove.
		write, /// Open existing file for read and write.
		WRITE = write, //TODO: deprecated, remove.
		create, /// Create new file and open it for read and write. If file exists it will be replaced by empty file.
		CREATE = create //TODO: deprecated, remove.
	};

	//TODO: deprecated, remove.
	typedef mode mode;

protected:
	mode ioMode; // mode only matters when file is opened

	/**
	 * @brief Constructor.
	 * @param pathName - initial path to set to the newly created file instance.
	 */
	file(const std::string& pathName = std::string()) :
			path_var(pathName)
	{}

public:
	file(const file&) = delete;
	file(file&&) = delete;
	file& operator=(const file&) = delete;

	/**
	 * @brief Destructor.
	 * This destructor does not call Close() method, but it has an ASSERT which checks if the file is closed.
	 * The file shall be closed upon the object destruction, all the implementations should
	 * assure that.
	 */
	virtual ~file()noexcept{
		ASSERT(!this->isOpened())
	}

	/**
	 * @brief Set the path for this file instance.
	 * @param pathName - the path to a file or directory.
	 */
	void setPath(const std::string& pathName)const{
		if(this->isOpened()){
			throw utki::invalid_state("papki::file::set_path(): Cannot set path when file is opened");
		}

		this->setPathInternal(pathName);
	}

protected:
	virtual void setPathInternal(const std::string& pathName)const{
		this->path_var = pathName;
	}
	
public:
	
	/**
	 * @brief Get the current path being held by this file instance.
	 * @return The path this file instance holds.
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
	 * @param io_mode - file opening mode (reading/writing/create).
	 * @throw utki::invalid_state - if file is already opened.
	 */
	void open(mode io_mode){
		if(this->isOpened()){
			throw utki::invalid_state("papki::file::open(): file is already opened");
		}
		if(this->isDir()){
			throw utki::invalid_state("file refers to directory. Directory cannot be opened.");
		}
		this->openInternal(io_mode);
		
		//set open mode
		if(io_mode == mode::create){
			this->ioMode = mode::write;
		}else{
			this->ioMode = io_mode;
		}

		this->isOpened_var = true;
		
		this->curPos_var = 0;
	};
	
	/**
	 * @brief Open file for reading.
	 * This is equivalent to Open(mode::read);
	 * @throw utki::invalid_state - if file is already opened.
	 */
	void open()const{
		const_cast<file*>(this)->open(mode::read);
	}
	
protected:
	/**
	 * @brief Open file, internal implementation.
	 * Derived class should override this function with its own implementation.
	 * @param mode - opening mode.
	 */
	virtual void openInternal(mode io_mode) = 0;
	
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
	 * If this file instance holds a path to a directory then this method
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
	 * @throw utki::invalid_state - if file is not opened.
	 */
	size_t read(utki::span<std::uint8_t> buf)const;

protected:
	/**
	 * @brief Read data from file.
	 * Override this function to implement reading routine. This function is called
	 * by Read() method after it has done some safety checks.
	 * It is assumed that the whole passed buffer needs to be filled with data.
	 * @param buf - buffer to fill with read data.
	 * @return number of bytes actually read.
	 */
	virtual size_t readInternal(utki::span<std::uint8_t> buf)const{
		throw std::runtime_error("readInternal(): unsupported");
	}
	
public:
	/**
	 * @brief Write data to file.
	 * Not all file systems support writing to a file, some file systems are read-only.
	 * @param buf - buffer holding the data to write.
	 * @return Number of bytes actually written. Normally, should always write all the passed data,
	 *         the only reasonable case when less data is written is when there is no more free space
	 *         in the file system.
	 * @throw utki::invalid_state - if file is not opened or opened for reading only.
	 */
	size_t write(const utki::span<std::uint8_t> buf);

protected:
	/**
	 * @brief Write data to file.
	 * Override this function to implement writing routine. This function is called
	 * by Write() method after it has done some safety checks.
	 * It is assumed that the whole passed buffer needs to be written to the file.
	 * @param buf - buffer containing the data to write.
	 * @return number of bytes actually written.
	 */
	virtual size_t writeInternal(const utki::span<std::uint8_t> buf){
		throw std::runtime_error("writeInternal(): unsupported");
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
	 * @throw utki::invalid_state - if file is not opened.
	 */
	size_t seekForward(size_t numBytesToSeek)const{
		if(!this->isOpened()){
			throw utki::invalid_state("seekForward(): file is not opened");
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
	 * @throw utki::invalid_state - if file is not opened.
	 */
	size_t seekBackward(size_t numBytesToSeek)const{
		if(!this->isOpened()){
			throw utki::invalid_state("seekBackward(): file is not opened");
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
		throw std::runtime_error("SeekBackward(): unsupported");
	}
	
public:

	/**
	 * @brief Seek to the beginning of the file.
	 * There is a default implementation of this operation by just closing and opening the file again.
	 * @throw utki::invalid_state - if file is not opened.
	 */
	void rewind()const{
		if(!this->isOpened()){
			throw utki::invalid_state("Rewind(): file is not opened");
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
		mode m = this->ioMode;
		this->close();
		const_cast<file*>(this)->open(m);
	}
	
public:

	/**
	 * @brief Create directory.
	 * If this file instance is a directory then try to create that directory on
	 * file system. Not all file systems are writable, so not all of them support
	 * directory creation.
	 * @throw utki::invalid_state - if file is opened.
	 */
	virtual void makeDir();

public:
	/**
	 * @brief Load the entire file into the RAM.
	 * @param maxBytesToLoad - maximum bytes to load. Default value is the maximum limit the size_t type can hold.
	 * @return Array containing loaded file data.
	 * @throw utki::invalid_state - if file is already opened.
	 */
	std::vector<std::uint8_t> loadWholeFileIntoMemory(size_t maxBytesToLoad = size_t(-1))const;

	/**
	 * @brief Check for file/directory existence.
	 * @return true - if file/directory exists.
	 * @return false - otherwise.
	 */
	virtual bool exists()const;

	
	/**
	 * @brief Creates another file object of same implementation.
	 * Creates a 'clone' of the file system implementation represented by this object.
	 * It does not copy current file path or open/close state etc. Spawned object is in
	 * initial state.
	 * @return Newly spawned file object.
	 */
	virtual std::unique_ptr<file> spawn() = 0;
	
	std::unique_ptr<const file> spawn()const{
		return const_cast<file*>(this)->spawn();
	}
	
public:
	/**
	 * @brief file guard class.
	 * Use this class to open the file within the particular scope.
	 * As the file guard object goes out of the scope it will close the file in its destructor.
	 * Usage:
	 * @code
	 *	file& fi;//assume we have some papki::file object visible in current scope.
	*	...
	*	{
	*		//assume the 'fi' is closed.
	*		//Let's create the file guard object. This will open the file 'fi'
	*		// for reading by calling fi.Open(papki::file::mode::read) method.
	*		papki::file::Guard fileGuard(fi, papki::file::mode::read);
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
	class guard{
		const file& f;
	public:
		guard(const file &file, mode mode);

		guard(const file &file);
		
		~guard();
	};

	//TODO: deprecated, remove.
	typedef guard Guard;
};

//TODO: deprecated, remove.
typedef file File;

}
