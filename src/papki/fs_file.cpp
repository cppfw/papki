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

#include <utki/config.hpp>
#include <utki/types.hpp>
#include <utki/util.hpp>

#if CFG_OS == CFG_OS_WINDOWS
#	include <utki/windows.hpp>
#elif CFG_OS == CFG_OS_LINUX || CFG_OS == CFG_OS_MACOSX
#	include <cerrno>
#	include <cstring>

#	include <dirent.h>
#	include <sys/stat.h>
#endif

#include <cstdlib>
#include <sstream>
#include <vector>

// on iOS we use 'dirent' instead of std::filesystem,
// see comment in fs_file::list_dir() function implementation for more details
#if CFG_OS_NAME != CFG_OS_NAME_IOS
#	include <filesystem>
#endif

#include "fs_file.hpp"

using namespace papki;

void fs_file::open_internal(mode mode)
{
	if (this->is_dir()) {
		throw std::logic_error("path refers to a directory, directories can't be opened");
	}

	const char* mode_str = [&mode]() {
		switch (mode) {
			case file::mode::write:
				return "r+b";
			case file::mode::create:
				return "w+b";
			case file::mode::read:
				return "rb";
			default:
				throw std::invalid_argument("unknown mode");
		}
	}();

#if CFG_COMPILER == CFG_COMPILER_MSVC
	if (fopen_s(&this->handle, this->path().c_str(), mode_str) != 0) {
		this->handle = 0;
	}
#else
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	this->handle = fopen(this->path().c_str(), mode_str);
#endif
	if (!this->handle) {
		LOG([&](auto& o) {
			o << "fs_file::open(): path() = " << this->path().c_str() << std::endl;
		})
		std::stringstream ss;
		ss << "fopen(" << this->path().c_str() << ") failed";
		throw std::system_error(errno, std::generic_category(), ss.str());
	}
}

void fs_file::close_internal() const noexcept
{
	ASSERT(this->handle)

	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	fclose(this->handle);
	this->handle = nullptr;
}

size_t fs_file::read_internal(utki::span<uint8_t> buf) const
{
	ASSERT(this->handle)
	size_t num_bytes_read = fread(buf.begin(), 1, buf.size(), this->handle);
	if (num_bytes_read != buf.size()) { // something happened
		if (!feof(this->handle)) {
			throw std::runtime_error("fread() error"); // if it is not an EndOfFile then it is error
		}
	}
	return num_bytes_read;
}

size_t fs_file::write_internal(utki::span<const uint8_t> buf)
{
	ASSERT(this->handle)
	size_t bytes_written = fwrite(buf.begin(), 1, buf.size(), this->handle);
	if (bytes_written != buf.size()) { // something bad has happened
		throw std::runtime_error("fwrite error");
	}

	return bytes_written;
}

size_t fs_file::seek_backward_internal(size_t num_bytes_to_seek) const
{
	ASSERT(this->handle)

	// NOTE: fseek() accepts 'long int' as offset argument which is signed and can
	// be
	//       less than size_t value passed as argument to this function.
	//       Therefore, do several seek operations with smaller offset if
	//       necessary.

	using fseek_offset_type = long int;
	const auto max_offset = size_t(((unsigned long int)(-1)) >> 1);
	ASSERT((size_t(1) << ((sizeof(fseek_offset_type) * 8) - 1)) - 1 == max_offset)
	static_assert(size_t(-(-fseek_offset_type(max_offset))) == max_offset, "error");

	using std::min;
	num_bytes_to_seek = min(num_bytes_to_seek, this->cur_pos()); // clamp top

	for (size_t num_bytes_left = num_bytes_to_seek; num_bytes_left != 0;) {
		ASSERT(num_bytes_left <= num_bytes_to_seek)

		fseek_offset_type offset = 0;
		if (num_bytes_left > max_offset) {
			offset = fseek_offset_type(max_offset);
		} else {
			offset = fseek_offset_type(num_bytes_left);
		}

		ASSERT(offset > 0)

		if (fseek(this->handle, -offset, SEEK_CUR) != 0) {
			throw std::runtime_error("fseek() failed");
		}

		ASSERT(size_t(offset) < size_t(-1))
		ASSERT(num_bytes_left >= size_t(offset))

		num_bytes_left -= size_t(offset);
	}

	return num_bytes_to_seek;
}

void fs_file::rewind_internal() const
{
	if (!this->is_open()) {
		throw std::logic_error("cannot rewind, file is not opened");
	}

	ASSERT(this->handle)
	if (fseek(this->handle, 0, SEEK_SET) != 0) {
		throw std::runtime_error("fseek() failed");
	}
}

bool fs_file::exists() const
{
	if (this->is_open()) { // file is opened => it exists
		return true;
	}

	if (this->path().size() == 0) {
		return false;
	}

	if (this->is_dir()) {
#if CFG_OS == CFG_OS_LINUX || CFG_OS == CFG_OS_MACOSX
		DIR* pdir = opendir(this->path().c_str());
		if (!pdir) {
			return false;
		} else {
			closedir(pdir);
			return true;
		}
#elif CFG_OS == CFG_OS_WINDOWS
		DWORD attrs = GetFileAttributesA(this->path().c_str());
		if (attrs == INVALID_FILE_ATTRIBUTES) {
			// Could not get file attributes, perhaps the file/directory does not
			// exist.
			return false;
		}

		if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
			// This is a directory and it exists
			return true;
		}

		return false;
#else
		throw std::runtime_exception("Checking for directory existence is not supported");
#endif
	} else {
		return this->file::exists();
	}
}

void fs_file::make_dir()
{
	if (this->is_open()) {
		throw std::logic_error("cannot make directory when file is opened");
	}

	if (this->path().size() == 0 || this->path()[this->path().size() - 1] != '/') {
		throw std::logic_error("invalid directory name, should end with '/'");
	}

#if CFG_OS == CFG_OS_LINUX || CFG_OS == CFG_OS_MACOSX
	umask(0); // clear umask for proper permissions of newly created directory
	constexpr auto default_permissions = 0755;
	if (mkdir(this->path().c_str(), default_permissions) != 0) {
		throw std::system_error(errno, std::generic_category(), "mkdir() failed");
	}
#elif CFG_OS == CFG_OS_WINDOWS
	if (!CreateDirectoryA(this->path().c_str(), nullptr)) {
		auto error = GetLastError();
		if (error != ERROR_ALREADY_EXISTS) {
			throw std::system_error(int(error), std::generic_category(), "CreateDirectory() failed");
		}
	}
#else
	throw std::runtime_error("creating directory is not supported");
#endif
}

std::string fs_file::get_home_dir()
{
	std::string ret;

#if CFG_OS == CFG_OS_WINDOWS && CFG_COMPILER == CFG_COMPILER_MSVC
	{
		char* buf = nullptr;
		size_t size = 0;

		utki::scope_exit scopeExit([&buf]() {
			free(buf);
		});

		if (_dupenv_s(&buf, &size, "USERPROFILE") != 0) {
			throw std::runtime_error("USERPROFILE  environment virable could not be read");
		}
		ret = std::string(buf);
	}
#elif CFG_OS == CFG_OS_LINUX || CFG_OS == CFG_OS_WINDOWS || CFG_OS == CFG_OS_MACOSX

#	if CFG_OS == CFG_OS_LINUX || CFG_OS == CFG_OS_MACOSX
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	char* home = getenv("HOME");
#	elif CFG_OS == CFG_OS_WINDOWS
	char* home = getenv("USERPROFILE");
#	else
#		error "unsupported OS"
#	endif

	if (!home) {
		throw std::runtime_error("HOME environment variable does not exist");
	}

	ret = std::string(home);
#else
#	error "unsupported os"
#endif

	// append trailing '/' if needed
	if (ret.size() == 0 || ret[ret.size() - 1] != '/') {
		ret += '/';
	}

	return ret;
}

std::vector<std::string> fs_file::list_dir(size_t max_size) const
{
	if (!this->is_dir()) {
		throw std::logic_error("fs_file::list_dir(): this is not a directory");
	}

	std::vector<std::string> files;

// For all systems except iOS we use new implementation via std::filesystem.
// On iOS the std::filesystem is available only starting from iOS 13.0 while it
// is still desired to support iOS 11.0 at least, so for iOS we fall back to old
// implementation via 'dirent.h'.
#if CFG_OS_NAME != CFG_OS_NAME_IOS
	std::filesystem::directory_iterator iter(this->path());

	for (const auto& p : iter) {
		std::string name = papki::not_dir(p.path().string());

		if (p.is_directory()) {
			name += "/";
		}

		files.push_back(name);
		if (files.size() == max_size) {
			break;
		}
	}
#else
	// Old implementation, used before std::filesystem became available. The code
	// is still kept here because std::filesystem support is not very common yet
	// and on some systems it might be needed to revert back to this old
	// implementation.
#	if CFG_OS == CFG_OS_WINDOWS
	{
		std::string pattern = this->path();
		pattern += '*';

		LOG([&](auto& o) {
			o << "fs_file::list_dir(): pattern = " << pattern << std::endl;
		})

		WIN32_FIND_DATA wfd;
		HANDLE h = FindFirstFile(pattern.c_str(), &wfd);
		if (h == INVALID_HANDLE_VALUE) {
			throw std::system_error(GetLastError(), std::generic_category(), "FindFirstFile() failed");
		}

		// create Find Closer to automatically call FindClose on exit from the
		// function in case of exceptions etc...
		{
			utki::scope_exit scopeExit([h]() {
				FindClose(h);
			});

			do {
				std::string s(wfd.cFileName);
				ASSERT(s.size() > 0)

				// do not add ./ and ../ directories, we are not interested in them
				if (s == "." || s == "..") {
					continue;
				}

				if (((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) && s[s.size() - 1] != '/') {
					s += '/';
				}
				files.push_back(s);

				if (files.size() == max_size) {
					break;
				}
			} while (FindNextFile(h, &wfd) != 0);

			auto error = GetLastError();

			if (error != ERROR_SUCCESS && error != ERROR_NO_MORE_FILES) {
				throw std::system_error(error, std::generic_category(), "list_dir(): find next file failed");
			}
		}
	}
#	elif CFG_OS == CFG_OS_LINUX || CFG_OS == CFG_OS_MACOSX
	{
		DIR* pdir = opendir(this->path().c_str());

		if (!pdir) {
			std::stringstream ss;
			ss << "fs_file::list_dir(): opendir() failure, error code = " << strerror(errno);
			throw std::system_error(errno, std::generic_category(), ss.str());
		}

		// create DirentCloser to automatically call closedir on exit from the
		// function in case of exceptions etc...
		struct DirCloser {
			DIR* pdir;

			DirCloser(DIR* pDirToClose) :
				pdir(pDirToClose)
			{}

			~DirCloser()
			{
				int ret;
				do {
					ret = closedir(this->pdir);
					ASSERT(ret == 0 || errno == EINTR, [](auto& o) {
						o << "fs_file::list_dir(): closedir() failed: " << strerror(errno);
					})
				} while (ret != 0 && errno == EINTR);
			}
		} dirCloser(pdir);

		errno = 0;
		while (dirent* pent = readdir(pdir)) {
			std::string s(pent->d_name);
			if (s == "." || s == "..")
				continue; // do not add ./ and ../ directories, we are not interested in
						  // them

			struct stat fileStats;
			if (stat((this->path() + s).c_str(), &fileStats) < 0) {
				std::stringstream ss;
				ss << "fs_file::list_dir(): stat() failure, error code = " << strerror(errno);
				throw std::system_error(errno, std::system_category(), ss.str());
			}

			if (fileStats.st_mode & S_IFDIR) // if this entry is a directory append '/' symbol to its end
				s += "/";

			files.push_back(s);

			if (files.size() == max_size) {
				break;
			}
		}

		// check if we exited the while() loop because of readdir() failed
		if (errno != 0) {
			std::stringstream ss;
			ss << "fs_file::list_dir(): readdir() failure, error code = " << strerror(errno);
			throw std::system_error(errno, std::system_category(), ss.str());
		}
	}
#	else
#		error "fs_file::list_dir(): is not implemented yet for this os"
#	endif
#endif

	return files;
}

uint64_t fs_file::size() const
{
	if (this->is_dir()) {
		throw std::logic_error("method size() is called on directory");
	}

#if CFG_OS == CFG_OS_WINDOWS
	HANDLE hfile = CreateFileA(
		this->path().c_str(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	if (hfile == INVALID_HANDLE_VALUE) {
		throw std::system_error(int(GetLastError()), std::generic_category(), "OpenFile() failed");
	}
	utki::scope_exit hfile_scope_exit([&hfile]() {
		if (CloseHandle(hfile) == 0) {
			LOG([](auto& o) {
				o << "error: CloseHandle() failed" << std::endl;
			})
		}
	});
	LARGE_INTEGER size;
	if (GetFileSizeEx(hfile, &size) == 0) {
		throw std::system_error(int(GetLastError()), std::generic_category(), "GetFileSizeEx() failed");
	}
	return size.QuadPart;
#elif CFG_OS == CFG_OS_LINUX || CFG_OS == CFG_OS_MACOSX
	struct stat file_stats{};

	if (stat(this->path().c_str(), &file_stats) < 0) {
		throw std::system_error(errno, std::system_category(), "stat() failed");
	}
	return file_stats.st_size;
#else
	return this->file::size();
#endif
}

std::unique_ptr<file> fs_file::spawn()
{
	return std::make_unique<fs_file>();
}
