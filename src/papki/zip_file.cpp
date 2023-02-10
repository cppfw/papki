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

#include "zip_file.hpp"

#include <sstream>

#include "unzip/unzip.hxx"

using namespace papki;

namespace {

voidpf ZCALLBACK UnzipOpen(voidpf opaque, const char* filename, int mode)
{
	papki::file* f = reinterpret_cast<papki::file*>(opaque);

	switch (mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) {
		case ZLIB_FILEFUNC_MODE_READ:
			f->open(papki::file::mode::read);
			break;
		default:
			throw std::invalid_argument(
				"UnzipOpen(): tried opening zip file something "
				"else than READ. Only READ is supported."
			);
	}

	return f;
}

int ZCALLBACK UnzipClose(voidpf opaque, voidpf stream)
{
	papki::file* f = reinterpret_cast<papki::file*>(stream);
	f->close();
	return 0;
}

uLong ZCALLBACK UnzipRead(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	papki::file* f = reinterpret_cast<papki::file*>(stream);
	return uLong(f->read(utki::span<uint8_t>(reinterpret_cast<uint8_t*>(buf), size)));
}

uLong ZCALLBACK UnzipWrite(voidpf opaque, voidpf stream, const void* buf, uLong size)
{
	ASSERT(false, [](auto& o) {
		o << "Writing ZIP files is not supported";
	})
	return 0;
}

int ZCALLBACK UnzipError(voidpf opaque, voidpf stream)
{
	return 0; // no error
}

long ZCALLBACK UnzipSeek(voidpf opaque, voidpf stream, uLong offset, int origin)
{
	papki::file* f = reinterpret_cast<papki::file*>(stream);

	// assume that offset can only be positive, since its type is unsigned

	switch (origin) {
		case ZLIB_FILEFUNC_SEEK_CUR:
			f->seek_forward(offset);
			return 0;
		case ZLIB_FILEFUNC_SEEK_END:
			f->seek_forward(std::numeric_limits<size_t>::max());
			return 0;
		case ZLIB_FILEFUNC_SEEK_SET:
			f->rewind();
			f->seek_forward(offset);
			return 0;
		default:
			return -1;
	}
}

long ZCALLBACK UnzipTell(voidpf opaque, voidpf stream)
{
	papki::file* f = reinterpret_cast<papki::file*>(stream);
	return long(f->cur_pos());
}

} // namespace

zip_file::zip_file(std::unique_ptr<papki::file> underlying_zip_file, std::string_view path) :
	papki::file(path),
	underlying_zip_file(std::move(underlying_zip_file))
{
	zlib_filefunc_def ff;
	ff.opaque = this->underlying_zip_file.operator->();
	ff.zopen_file = &UnzipOpen;
	ff.zclose_file = &UnzipClose;
	ff.zread_file = &UnzipRead;
	ff.zwrite_file = &UnzipWrite;
	ff.zseek_file = &UnzipSeek;
	ff.zerror_file = &UnzipError;
	ff.ztell_file = &UnzipTell;

	this->handle = unzOpen2(this->underlying_zip_file->path().c_str(), &ff);

	if (!this->handle) {
		throw std::runtime_error("zip_file: opening zip file failed");
	}
}

zip_file::~zip_file() noexcept
{
	this->close(); // make sure there is no file opened inside zip file

	if (unzClose(this->handle) != UNZ_OK) {
		ASSERT(false)
	}
}

void zip_file::open_internal(mode mode)
{
	if (mode != file::mode::read) {
		throw std::invalid_argument("illegal mode requested, only READ supported inside ZIP file");
	}

	if (unzLocateFile(this->handle, this->path().c_str(), 0) != UNZ_OK) {
		std::stringstream ss;
		ss << "zip_file::OpenInternal(): file not found: " << this->path();
		throw std::runtime_error(ss.str());
	}

	{
		unz_file_info zip_file_info;

		if (unzGetCurrentFileInfo(this->handle, &zip_file_info, 0, 0, 0, 0, 0, 0) != UNZ_OK) {
			throw std::runtime_error("failed obtaining file info");
		}
	}

	if (unzOpenCurrentFile(this->handle) != UNZ_OK) {
		throw std::runtime_error("file opening failed");
	}
}

void zip_file::close_internal() const noexcept
{
	if (unzCloseCurrentFile(this->handle) == UNZ_CRCERROR) {
		ASSERT(false, [](auto& o) {
			o << "zip_file::close(): CRC is not good" << std::endl;
		})
	}
}

size_t zip_file::read_internal(utki::span<uint8_t> buf) const
{
	ASSERT(buf.size() <= unsigned(-1))
	int num_bytes_read = unzReadCurrentFile(this->handle, buf.begin(), unsigned(buf.size()));
	if (num_bytes_read < 0) {
		throw std::runtime_error("zip_file::Read(): file reading failed");
	}

	ASSERT(num_bytes_read >= 0)
	return size_t(num_bytes_read);
}

bool zip_file::exists() const
{
	if (this->is_dir()) {
		return this->file::exists();
	}

	if (this->path().size() == 0) {
		return false;
	}

	if (this->is_open()) {
		return true;
	}

	return unzLocateFile(this->handle, this->path().c_str(), 0) == UNZ_OK;
}

std::vector<std::string> zip_file::list_dir(size_t max_entries) const
{
	if (!this->is_dir()) {
		throw std::logic_error("zip_file::list_dir(): this is not a directory");
	}

	// if path refers to directory then there should be no files opened
	ASSERT(!this->is_open())

	if (!this->handle) {
		throw std::logic_error("zip_file::list_dir(): zip file is not opened");
	}

	std::vector<std::string> files;

	// for every file, check if it is in the current directory
	{
		// move to first file
		int ret = unzGoToFirstFile(this->handle);

		if (ret != UNZ_OK) {
			throw std::runtime_error("zip_file::list_dir(): unzGoToFirstFile() failed.");
		}

		do {
			std::array<char, 255> file_name_buf;

			if (unzGetCurrentFileInfo(
					this->handle,
					NULL,
					&*file_name_buf.begin(),
					uLong(file_name_buf.size()),
					NULL,
					0,
					NULL,
					0
				)
				!= UNZ_OK)
			{
				throw std::runtime_error("zip_file::list_dir(): unzGetCurrentFileInfo() failed.");
			}

			file_name_buf[file_name_buf.size() - 1] = 0; // null-terminate, just to be on a safe side

			std::string filename(file_name_buf.data());

			if (filename.size() <= this->path().size()) {
				continue;
			}

			// check if full file path starts with the this->path() string
			const std::string& cur_path =
				this->path().compare(0, 2, "./") == 0 ? this->path().substr(2) : this->path(); // remove leading "./"
			if (filename.compare(0, cur_path.size(), cur_path) != 0) {
				continue;
			}

			ASSERT(filename.size() > cur_path.size())
			std::string subfilename(filename, cur_path.size(), filename.size() - cur_path.size());

			size_t slash_pos = subfilename.find_first_of('/');

			if (slash_pos == std::string::npos) {
				files.push_back(subfilename);
				continue;
			}

			// if we get here then we need to add a directory
			ASSERT(subfilename.size() >= slash_pos + 1)
			if (slash_pos == subfilename.size() - 1) {
				files.push_back(std::move(subfilename));
			}

			if (files.size() == max_entries) {
				break;
			}
		} while ((ret = unzGoToNextFile(this->handle)) == UNZ_OK);

		if (ret != UNZ_END_OF_LIST_OF_FILE && ret != UNZ_OK) {
			throw std::runtime_error("zip_file::list_dir(): unzGoToNextFile() failed.");
		}
	}

	return files;
}
