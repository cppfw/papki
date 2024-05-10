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

#pragma once

#include <memory>
#include <stdexcept>
#include <string>

#include <utki/config.hpp>
#include <utki/debug.hpp>
#include <utki/span.hpp>

#include "util.hpp"

#ifdef assert
#	undef assert
#endif

namespace papki {

/**
 * @brief Abstract interface to a file system.
 * This class represents an abstract interface to a file system.
 */
class file
{
	mutable std::string cur_path;

	mutable bool is_file_open = false;

	mutable size_t current_pos = 0; // holds current position from file beginning

public:
	/**
	 * @brief Modes of opening the file.
	 */
	enum class mode {
		read, /// Open existing file for read only.
		write, /// Open existing file for read and write.
		create /// Create new file and open it for read and write. If file exists it
			   /// will be replaced by empty file.
	};

protected:
	mode io_mode = mode::read; // mode only matters when file is opened

	/**
	 * @brief Constructor.
	 * @param path_name - initial path to set to the newly created file instance.
	 */
	file(std::string_view path_name = std::string_view()) :
		cur_path(path_name)
	{}

public:
	file(const file&) = delete;
	file& operator=(const file&) = delete;

	file(file&&) = delete;
	file& operator=(file&&) = delete;

	/**
	 * @brief Destructor.
	 * This destructor does not call Close() method, but it has an ASSERT which
	 * checks if the file is closed. The file shall be closed upon the object
	 * destruction, all the implementations should assure that.
	 */
	// NOLINTNEXTLINE(modernize-use-equals-default, "the destructor is not trivial in DEBUG build config")
	virtual ~file() noexcept
	{
		ASSERT(!this->is_open())
	}

	/**
	 * @brief Set the path for this file instance.
	 * @param path_name - the path to a file or directory.
	 * @return Reference to this file object.
	 */
	const file& set_path(std::string path_name) const
	{
		if (this->is_open()) {
			throw std::logic_error("papki::file::set_path(): cannot set path when file is open");
		}

		this->set_path_internal(std::move(path_name));

		return *this;
	}

	/**
	 * @brief Set the path for this file instance.
	 * @param path_name - the path to a file or directory.
	 * @return Reference to this file object.
	 */
	const file& set_path(std::string_view path_name) const
	{
		return this->set_path(std::string(path_name));
	}

	/**
	 * @brief Set the path for this file instance.
	 * @param path_name - the path to a file or directory.
	 * @return Reference to this file object.
	 */
	const file& set_path(const char* path_name) const
	{
		return this->set_path(std::string(path_name));
	}

protected:
	virtual void set_path_internal(std::string&& path_name) const
	{
		this->cur_path = std::move(path_name);
	}

public:
	/**
	 * @brief Get the current path being held by this file instance.
	 * @return The path this file instance holds.
	 */
	const std::string& path() const noexcept
	{
		return this->cur_path;
	}

	/**
	 * @brief Get current position from beginning of the file.
	 * @return Current position from beginning of the file
	 */
	size_t cur_pos() const noexcept
	{
		return this->current_pos;
	}

	/**
	 * @brief Get file suffix.
	 * Returns a string containing the tail part of the file path, everything that
	 * goes after the last dot character ('.') in the file path string.
	 * I.e. if the file path is '/home/user/some.file.txt' then the return value
	 * will be 'txt'.
	 * Note, that on *nix systems if the file name starts with a dot then this
	 * file is treated as hidden, in that case it is thought that the file has no
	 * suffix. I.e., for example , if the file path is '/home/user/.myfile' then
	 * the file has no suffix and this function will return an empty string.
	 * Although, if the file path is '/home/user/.myfile.txt' then the file does
	 * have a suffix and the function will return 'txt'.
	 * @return String representing file suffix.
	 */
	std::string suffix() const
	{
		return papki::suffix(this->path());
	}

	/**
	 * @brief Get file name without suffix.
	 * Returns a string containing the file path without suffix, everything that
	 * goes after the last dot character ('.') is trimmed, including the dot
	 * character. I.e. if the file path is '/home/user/some.file.txt' then the
	 * return value will be '/home/user/some.file'. Note, that on *nix systems if
	 * the file name starts with a dot then this file is treated as hidden, in
	 * that case it is thought that the file has no suffix. I.e., for example , if
	 * the file path is '/home/user/.myfile' then the file has no suffix and this
	 * function will return same string, i.e. '/home/user/.myfile'. Although, if
	 * the file path is '/home/user/.myfile.txt' then the file does have a suffix
	 * and the function will return '/home/user/.myfile'.
	 * @return String representing file name without suffix.
	 */
	std::string not_suffix()
	{
		return papki::not_suffix(this->path());
	}

	/**
	 * @brief Get directory part of the path.
	 * Example: if path is '/home/user/some.file.txt' then the return value
	 * will be '/home/user/'.
	 * @return String representation of directory part of the path.
	 */
	std::string dir() const
	{
		return papki::dir(this->path());
	}

	/**
	 * @brief Get file part of the path.
	 * Example: if path is '/home/user/some.file.txt' then the return value
	 * will be 'some.file.txt'.
	 * @return String representation of directory part of the path.
	 */
	std::string not_dir() const
	{
		return papki::not_dir(this->path());
	}

	/**
	 * @brief Open file.
	 * Opens file for reading/writing or creates the file.
	 * @param io_mode - file opening mode (reading/writing/create).
	 * @throw std::logic_error - if file is already opened.
	 */
	void open(mode io_mode);

	/**
	 * @brief Open file for reading.
	 * This is the equivalent to open(mode::read);
	 * @throw std::logic_error - if file is already opened.
	 */
	void open() const
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
		const_cast<file*>(this)->open(mode::read);
	}

protected:
	/**
	 * @brief Open file, internal implementation.
	 * Derived class should override this function with its own implementation.
	 * @param io_mode - opening mode.
	 */
	virtual void open_internal(mode io_mode) = 0;

public:
	/**
	 * @brief Close file.
	 */
	void close() const noexcept;

protected:
	/**
	 * @brief Close file, internal implementation.
	 * Derived class should override this function with its own implementation.
	 */
	virtual void close_internal() const noexcept = 0;

public:
	/**
	 * @brief Check if the file is open.
	 * @return true - if the file is open.
	 * @return false - otherwise.
	 */
	bool is_open() const noexcept
	{
		return this->is_file_open;
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
	bool is_dir() const noexcept;

	/**
	 * @brief Get list of files and subdirectories of a directory.
	 * If this file instance holds a path to a directory then this method
	 * can be used to obtain the contents of the directory.
	 * @param max_size - maximum size of the returned list.
	 * @return The array of string objects representing the directory entries.
	 */
	virtual std::vector<std::string> list_dir(size_t max_size = std::numeric_limits<size_t>::max()) const;

	/**
	 * @brief Read data from file.
	 * All sane file systems should support file reading.
	 * Returns number of bytes actually read. It always reads the requested number
	 * of bytes, unless end of file reached, in which case the return value will
	 * be less than number of bytes to read was requested by argument.
	 * @param buf - buffer where to store the read data.
	 * @return Number of bytes actually read. Shall always be equal to number of
	 * bytes requested to read except the case when end of file reached.
	 * @throw std::logic_error - if file is not opened.
	 */
	size_t read(utki::span<uint8_t> buf) const;

protected:
	/**
	 * @brief Read data from file.
	 * Override this function to implement reading routine. This function is
	 * called by read() method after it has done some safety checks. It is assumed
	 * that the whole passed buffer needs to be filled with data.
	 * @param buf - buffer to fill with read data.
	 * @return number of bytes actually read.
	 */
	virtual size_t read_internal(utki::span<uint8_t> buf) const
	{
		throw std::runtime_error("readInternal(): unsupported");
	}

public:
	/**
	 * @brief Write data to file.
	 * Not all file systems support writing to a file, some file systems are
	 * read-only.
	 * @param buf - buffer holding the data to write.
	 * @return Number of bytes actually written. Normally, should always write all
	 * the passed data, the only reasonable case when less data is written is when
	 * there is no more free space in the file system.
	 */
	size_t write(utki::span<const uint8_t> buf);

	/**
	 * @brief Write data to file.
	 * Same as write(span<uint8_t>) but for span of chars.
	 * @param buf - buffer holding the data to write.
	 * @return Number of bytes actually written. Normally, should always write all
	 * the passed data, the only reasonable case when less data is written is when
	 * there is no more free space in the file system.
	 */
	size_t write(utki::span<const char> buf)
	{
		return this->write(
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			utki::make_span(reinterpret_cast<const uint8_t*>(buf.data()), buf.size())
		);
	}

	/**
	 * @brief Write string to a file.
	 * Same as write(span<const char>) but for string.
	 * @param str - string to write to a file.
	 * @return Number of bytes actually written. Normally, should always write all
	 * the passed data, the only reasonable case when less data is written is when
	 * there is no more free space in the file system.
	 */
	size_t write(const std::string& str)
	{
		return this->write(utki::make_span(str));
	}

protected:
	/**
	 * @brief Write data to file.
	 * Override this function to implement writing routine. This function is
	 * called by Write() method after it has done some safety checks. It is
	 * assumed that the whole passed buffer needs to be written to the file.
	 * @param buf - buffer containing the data to write.
	 * @return number of bytes actually written.
	 */
	virtual size_t write_internal(utki::span<const uint8_t> buf)
	{
		throw std::runtime_error("write_internal(): unsupported");
	}

public:
	/**
	 * @brief Seek forward.
	 * Seek file pointer forward relatively to current position.
	 * There is a default implementation of this function which uses read() method
	 * to skip the specified number of bytes by reading the data and wasting it
	 * away. It will not go beyond the end of file.
	 * @param num_bytes_to_seek - number of bytes to skip.
	 * @return number of bytes actually skipped.
	 * @throw std::logic_error - if file is not opened.
	 */
	size_t seek_forward(size_t num_bytes_to_seek) const;

protected:
	/**
	 * @brief Seek forward, internal implementation.
	 * This function is called by seek_forward() after it has done some safety
	 * checks. Derived class may override this function with its own
	 * implementation. Otherwise, there is a default implementation which just
	 * reads and wastes necessary amount of bytes.
	 * @param num_bytes_to_seek - number of bytes to seek.
	 * @return number of bytes actually skipped.
	 */
	virtual size_t seek_forward_internal(size_t num_bytes_to_seek) const;

public:
	/**
	 * @brief Seek backwards.
	 * Seek file pointer backwards relatively to he current position. Not all file
	 * systems support seeking backwards.
	 * @param num_bytes_to_seek - number of bytes to skip.
	 * @return number of bytes actually skipped.
	 */
	size_t seek_backward(size_t num_bytes_to_seek) const;

protected:
	/**
	 * @brief Seek backwards, internal implementation.
	 * This function is called by seek_backward() after it has done some safety
	 * checks. Derived class may override this function with its own
	 * implementation.
	 * @param num_bytes_to_seek - number of bytes to seek.
	 * @return number of bytes actually skipped.
	 */
	virtual size_t seek_backward_internal(size_t num_bytes_to_seek) const;

public:
	/**
	 * @brief Seek to the beginning of the file.
	 * There is a default implementation of this operation by just closing and
	 * opening the file again.
	 * @throw std::logic_error - if file is not opened.
	 */
	void rewind() const;

protected:
	/**
	 * @brief Rewind, internal implementation.
	 * This function is called by rewind() after it has done some safety checks.
	 * Derived class may override this function with its own implementation.
	 */
	virtual void rewind_internal() const;

public:
	/**
	 * @brief Create directory.
	 * If this file instance is a directory then try to create that directory on
	 * file system. Not all file systems are writable, so not all of them support
	 * directory creation.
	 * @throw std::logic_error - if file is opened.
	 */
	virtual void make_dir();

public:
	/**
	 * @brief Load the entire file into the RAM.
	 * @param max_bytes_to_load - maximum bytes to load. Default value is the
	 * maximum limit the size_t type can hold.
	 * @return Array containing loaded file data.
	 * @throw std::logic_error - if file is already opened.
	 */
	std::vector<uint8_t> load(size_t max_bytes_to_load = ~0) const;

	/**
	 * @brief Check for file/directory existence.
	 * @return true - if file/directory exists.
	 * @return false - otherwise.
	 */
	virtual bool exists() const;

	/**
	 * @brief Get file size.
	 * The file must not be open when calling this method.
	 * Not all file systems provide efficient way to get file size.
	 * Fall back implementation is to open the file, seek to its end and get
	 * current position.
	 * @return file size.
	 */
	virtual uint64_t size() const;

	/**
	 * @brief Creates another file object of same implementation.
	 * Creates a 'clone' of the file system implementation represented by this
	 * object. It does not copy current file path or open/close state etc. Spawned
	 * object is in initial state.
	 * @return Newly spawned file object.
	 */
	virtual std::unique_ptr<file> spawn() = 0;

	// NOTE: it must not be possible to modify the const file by spawning
	// non-const file
	//       object and setting the same path to it, so make const spawn()
	//       overloads
	std::unique_ptr<const file> spawn() const
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
		return const_cast<file*>(this)->spawn();
	}

	std::unique_ptr<file> spawn(std::string path);

	std::unique_ptr<const file> spawn(std::string path) const
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
		return const_cast<file*>(this)->spawn(std::move(path));
	}

	std::unique_ptr<const file> spawn(const std::string& path) const
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
		return const_cast<file*>(this)->spawn(std::string(path));
	}

public:
	/**
	 * @brief file guard class.
	 * Use this class to open the file within the particular scope.
	 * As the file guard object goes out of the scope it will close the file in
	 *its destructor. Usage:
	 * @code
	 *	file& fi; // assume we have some papki::file object visible in current
	 *scope.
	 *	...
	 *	{
	 *		// assume the 'fi' is closed.
	 *		// Let's create the file guard object. This will open the file
	 *'fi'
	 *		//  for reading by calling fi.open(papki::file::mode::read)
	 *method. papki::file::guard file_guard(fi, papki::file::mode::read);
	 *
	 *		...
	 *		// do some reading
	 *		fi.read(...);
	 *
	 *		// going out of scope will destroy the 'file_guard' object. In
	 *turn,
	 *		// it will automatically close the file 'fi' in its destructor
	 *by
	 *		// calling fi.close() method.
	 *	}
	 * @endcode
	 */
	class guard
	{
		const file& f;

	public:
		guard(const file& file, mode mode);

		guard(const file& file);

		guard(const guard&) = delete;
		guard& operator=(const guard&) = delete;

		guard(guard&&) = delete;
		guard& operator=(guard&&) = delete;

		~guard();
	};
};

} // namespace papki
