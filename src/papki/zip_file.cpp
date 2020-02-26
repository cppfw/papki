#include "zip_file.hpp"

#include "unzip/unzip.hxx"

#include <sstream>

using namespace papki;

namespace{

voidpf ZCALLBACK UnzipOpen(voidpf opaque, const char* filename, int mode){
	papki::file* f = reinterpret_cast<papki::file*>(opaque);
	
	switch(mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER){
		case ZLIB_FILEFUNC_MODE_READ:
			f->open(papki::file::mode::read);
			break;
		default:
			throw std::invalid_argument("UnzipOpen(): tried opening zip file something else than READ. Only READ is supported.");
	}
	
	return f;
}

int ZCALLBACK UnzipClose(voidpf opaque, voidpf stream){
	papki::file* f = reinterpret_cast<papki::file*>(stream);
	f->close();
	return 0;
}

uLong ZCALLBACK UnzipRead(voidpf opaque, voidpf stream, void* buf, uLong size){
	papki::file* f = reinterpret_cast<papki::file*>(stream);
	return uLong(f->read(utki::span<std::uint8_t>(reinterpret_cast<std::uint8_t*>(buf), size)));
}

uLong ZCALLBACK UnzipWrite(voidpf opaque, voidpf stream, const void* buf, uLong size){
	ASSERT_INFO(false, "Writing ZIP files is not supported")
	return 0;
}

int ZCALLBACK UnzipError(voidpf opaque, voidpf stream){
	return 0; // no error
}

long ZCALLBACK UnzipSeek(voidpf  opaque, voidpf stream, uLong offset, int origin){
	papki::file* f = reinterpret_cast<papki::file*>(stream);
	
	// assume that offset can only be positive, since its type is unsigned
	
	switch(origin){
		case ZLIB_FILEFUNC_SEEK_CUR:
			f->seekForward(offset);
			return 0;
		case ZLIB_FILEFUNC_SEEK_END:
			f->seekForward(size_t(-1));
			return 0;
		case ZLIB_FILEFUNC_SEEK_SET:
			f->rewind();
			f->seekForward(offset);
			return 0;
		default:
			return -1;
	}
}

long ZCALLBACK UnzipTell(voidpf opaque, voidpf stream){
	papki::file* f = reinterpret_cast<papki::file*>(stream);
	return long(f->curPos());
}

}

zip_file::zip_file(std::unique_ptr<papki::file> zipFile, const std::string& path) :
		papki::file(path),
		zipFile(std::move(zipFile))
{
	zlib_filefunc_def ff;
	ff.opaque = this->zipFile.operator->();
	ff.zopen_file = &UnzipOpen;
	ff.zclose_file = &UnzipClose;
	ff.zread_file = &UnzipRead;
	ff.zwrite_file = &UnzipWrite;
	ff.zseek_file = &UnzipSeek;
	ff.zerror_file = &UnzipError;
	ff.ztell_file = &UnzipTell;
	
	this->handle = unzOpen2(this->zipFile->path().c_str(), &ff);

	if(!this->handle){
		throw std::runtime_error("zip_file: opening zip file failed");
	}
}

zip_file::~zip_file()noexcept{
	this->close(); // make sure there is no file opened inside zip file

	if(unzClose(this->handle) != UNZ_OK){
		ASSERT(false)
	}
}

void zip_file::open_internal(mode mode) {
	if(mode != file::mode::read){
		throw std::invalid_argument("illegal mode requested, only READ supported inside ZIP file");
	}

	if(unzLocateFile(this->handle, this->path().c_str(), 0) != UNZ_OK){
		std::stringstream ss;
		ss << "zip_file::OpenInternal(): file not found: " << this->path();
		throw std::runtime_error(ss.str());
	}

	{
		unz_file_info zipFileInfo;

		if(unzGetCurrentFileInfo(this->handle, &zipFileInfo, 0, 0, 0, 0, 0, 0) != UNZ_OK){
			throw std::runtime_error("failed obtaining file info");
		}
	}

	if(unzOpenCurrentFile(this->handle) != UNZ_OK){
		throw std::runtime_error("file opening failed");
	}
}

void zip_file::close_internal()const noexcept{
	if(unzCloseCurrentFile(this->handle) == UNZ_CRCERROR){
		TRACE(<< "[WARNING] zip_file::Close(): CRC is not good" << std::endl)
		ASSERT(false)
	}
}

size_t zip_file::read_internal(utki::span<std::uint8_t> buf)const{
	ASSERT(buf.size() <= unsigned(-1))
	int numBytesRead = unzReadCurrentFile(this->handle, buf.begin(), unsigned(buf.size()));
	if(numBytesRead < 0){
		throw std::runtime_error("zip_file::Read(): file reading failed");
	}

	ASSERT(numBytesRead >= 0)
	return size_t(numBytesRead);
}

bool zip_file::exists()const{
	if(this->is_dir()){
		return this->file::exists();
	}
	
	if(this->path().size() == 0){
		return false;
	}

	if(this->isOpened()){
		return true;
	}
	
	return unzLocateFile(this->handle, this->path().c_str(), 0) == UNZ_OK;
}

std::vector<std::string> zip_file::list_dir(size_t maxEntries)const{
	if(!this->is_dir()){
		throw utki::invalid_state("zip_file::list_dir(): this is not a directory");
	}

	// if path refers to directory then there should be no files opened
	ASSERT(!this->is_open())

	if(!this->handle){
		throw utki::invalid_state("zip_file::list_dir(): zip file is not opened");
	}

	std::vector<std::string> files;

	// for every file, check if it is in the current directory
	{
		// move to first file
		int ret = unzGoToFirstFile(this->handle);

		if(ret != UNZ_OK){
			throw std::runtime_error("zip_file::list_dir(): unzGoToFirstFile() failed.");
		}

		do{
			std::array<char, 255> fileNameBuf;

			if(unzGetCurrentFileInfo(
					this->handle,
					NULL,
					&*fileNameBuf.begin(),
					uLong(fileNameBuf.size()),
					NULL,
					0,
					NULL,
					0
				) != UNZ_OK)
			{
				throw std::runtime_error("zip_file::list_dir(): unzGetCurrentFileInfo() failed.");
			}

			fileNameBuf[fileNameBuf.size() - 1] = 0; // null-terminate, just to be on a safe side

			std::string filename(fileNameBuf.data());

			if(filename.size() <= this->path().size()){
				continue;
			}

			// check if full file path starts with the this->path() string
			const std::string& cur_path = this->path().compare(0, 2, "./") == 0 ? this->path().substr(2) : this->path(); // remove leading "./"
			if(filename.compare(0, cur_path.size(), cur_path) != 0){
				continue;
			}

			ASSERT(filename.size() > cur_path.size())
			std::string subfilename(filename, cur_path.size(), filename.size() - cur_path.size());

			// TRACE(<< "subfilename = " << subfilename << std::endl)

			size_t slash_pos = subfilename.find_first_of('/');

			if(slash_pos == std::string::npos){
				files.push_back(subfilename);
				continue;
			}

			// if we get here then we need to add a directory
			ASSERT(subfilename.size() >= slash_pos + 1)
			if(slash_pos == subfilename.size() - 1){
				files.push_back(std::move(subfilename));
			}

			if(files.size() == maxEntries){
				break;
			}
		}while((ret = unzGoToNextFile(this->handle)) == UNZ_OK);

		if(ret != UNZ_END_OF_LIST_OF_FILE && ret != UNZ_OK){
			throw std::runtime_error("zip_file::list_dir(): unzGoToNextFile() failed.");
		}
	}

	return files;
}
