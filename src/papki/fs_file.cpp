#include <utki/config.hpp>
#include <utki/types.hpp>

#if M_OS == M_OS_WINDOWS
#	include <utki/windows.hpp>

#elif M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX
#	include <dirent.h>
#	include <sys/stat.h>
#	include <cerrno>
#	include <cstring>

#endif

#include <vector>
#include <cstdlib>
#include <sstream>

#include "fs_file.hpp"



using namespace papki;



void fs_file::open_internal(mode mode){
	if(this->isDir()){
		throw utki::invalid_state("path refers to a directory, directories can't be opened");
	}

	const char* modeStr;
	switch(mode){
		case File::mode::write:
			modeStr = "r+b";
			break;
		case File::mode::create:
			modeStr = "w+b";
			break;
		case File::mode::read:
			modeStr = "rb";
			break;
		default:
			throw std::invalid_argument("unknown mode");
	}

#if M_COMPILER == M_COMPILER_MSVC
	if(fopen_s(&this->handle, this->path().c_str(), modeStr) != 0){
		this->handle = 0;
	}
#else
	this->handle = fopen(this->path().c_str(), modeStr);
#endif
	if(!this->handle){
		TRACE(<< "fs_file::Open(): Path() = " << this->path().c_str() << std::endl)
		std::stringstream ss;
		ss << "fopen(" << this->path().c_str() << ") failed";
		throw std::system_error(errno, std::generic_category(), ss.str());
	}
}

void fs_file::close_internal()const noexcept{
	ASSERT(this->handle)

	fclose(this->handle);
	this->handle = 0;
}

size_t fs_file::readInternal(utki::span<std::uint8_t> buf)const{
	ASSERT(this->handle)
	size_t numBytesRead = fread(buf.begin(), 1, buf.size(), this->handle);
	if(numBytesRead != buf.size()){ // something happened
		if(!feof(this->handle)){
			throw std::runtime_error("fread() error");//if it is not an EndOfFile then it is error
		}
	}
	return numBytesRead;
}

size_t fs_file::writeInternal(const utki::span<std::uint8_t> buf){
	ASSERT(this->handle)
	size_t bytesWritten = fwrite(buf.begin(), 1, buf.size(), this->handle);
	if(bytesWritten != buf.size()){//something bad has happened
		throw std::runtime_error("fwrite error");
	}

	return bytesWritten;
}



size_t fs_file::seekBackwardInternal(size_t numBytesToSeek)const{
	ASSERT(this->handle)

	//NOTE: fseek() accepts 'long int' as offset argument which is signed and can be
	//      less than size_t value passed as argument to this function.
	//      Therefore, do several seek operations with smaller offset if necessary.

	typedef long int T_FSeekOffset;
	const std::size_t DMax = std::size_t(
			((unsigned long int)(-1)) >> 1
		);
	ASSERT((size_t(1) << ((sizeof(T_FSeekOffset) * 8) - 1)) - 1 == DMax)
	static_assert(size_t(-(-T_FSeekOffset(DMax))) == DMax, "error");

	utki::clampTop(numBytesToSeek, this->curPos());

	for(size_t numBytesLeft = numBytesToSeek; numBytesLeft != 0;){
		ASSERT(numBytesLeft <= numBytesToSeek)

		T_FSeekOffset offset;
		if(numBytesLeft > DMax){
			offset = T_FSeekOffset(DMax);
		}else{
			offset = T_FSeekOffset(numBytesLeft);
		}

		ASSERT(offset > 0)

		if(fseek(this->handle, -offset, SEEK_CUR) != 0){
			throw std::runtime_error("fseek() failed");
		}

		ASSERT(size_t(offset) < size_t(-1))
		ASSERT(numBytesLeft >= size_t(offset))

		numBytesLeft -= size_t(offset);
	}

	return numBytesToSeek;
}

void fs_file::rewindInternal()const{
	if(!this->isOpened()){
		throw utki::invalid_state("cannot rewind, file is not opened");
	}

	ASSERT(this->handle)
	if(fseek(this->handle, 0, SEEK_SET) != 0){
		throw std::runtime_error("fseek() failed");
	}
}

bool fs_file::exists()const{
	if(this->isOpened()){ //file is opened => it exists
		return true;
	}

	if(this->path().size() == 0){
		return false;
	}

	//if it is a directory, check directory existence
	if(this->path()[this->path().size() - 1] == '/'){
#if M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX
		DIR *pdir = opendir(this->path().c_str());
		if(!pdir){
			return false;
		}else{
			closedir(pdir);
			return true;
		}
#elif M_OS == M_OS_WINDOWS
		DWORD attrs = GetFileAttributesA(this->path().c_str());
		if (attrs == INVALID_FILE_ATTRIBUTES){
			//Could not get file attributes, perhaps the file/directory does not exist.
			return false;
		}

		if (attrs & FILE_ATTRIBUTE_DIRECTORY){
			//This is a directory and it exists
			return true;
		}

		return false;
#else
		throw papki::exception("Checking for directory existence is not supported");
#endif
	}else{
		return this->File::exists();
	}
}



void fs_file::makeDir(){
	if(this->isOpened()){
		throw utki::invalid_state("cannot make directory when file is opened");
	}

	if(this->path().size() == 0 || this->path()[this->path().size() - 1] != '/'){
		throw utki::invalid_state("invalid directory name, should end with '/'");
	}

#if M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX
//	TRACE(<< "creating directory = " << this->Path() << std::endl)
	umask(0);//clear umask for proper permissions of newly created directory
	if(mkdir(this->path().c_str(), 0755) != 0){
		throw std::system_error(errno, std::generic_category(), "mkdir() failed");
	}
#elif M_OS == M_OS_WINDOWS
	if (!CreateDirectory(this->path().c_str(), NULL)){
		auto error = GetLastError();
		if(error != ERROR_ALREADY_EXISTS){
			throw std::system_error(error, std::generic_category(), "CreateDirectory() failed");
		}
	}
#else
	throw std::runtime_error("creating directory is not supported");
#endif
}



std::string fs_file::getHomeDir() {
	std::string ret;

#if M_OS == M_OS_WINDOWS && M_COMPILER == M_COMPILER_MSVC
	{
		char* buf = nullptr;
		size_t size = 0;

		utki::ScopeExit scopeExit([&buf](){
			free(buf);
		});

		if (_dupenv_s(&buf, &size, "USERPROFILE") != 0) {
			throw papki::exception("USERPROFILE  environment virable could not be read");
		}
		ret = std::string(buf);
	}
#elif M_OS == M_OS_LINUX || M_OS == M_OS_WINDOWS || M_OS == M_OS_MACOSX

#	if M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX
	char * home = getenv("HOME");
#	elif M_OS == M_OS_WINDOWS
	char * home = getenv("USERPROFILE");
#	else
#		error "unsupported OS"
#	endif

	if(!home){
		throw std::runtime_error("HOME environment variable does not exist");
	}

	ret = std::string(home);
#else
#	error "unsupported os"
#endif

	//append trailing '/' if needed
	if(ret.size() == 0 || ret[ret.size() - 1] != '/'){
		ret += '/';
	}

	return ret;
}


std::vector<std::string> fs_file::list_dir(size_t maxEntries)const{
	if(!this->isDir()){
		throw utki::invalid_state("fs_file::list_dir(): this is not a directory");
	}

	std::vector<std::string> files;

#if M_OS == M_OS_WINDOWS
	{
		std::string pattern = this->path();
		pattern += '*';

		TRACE(<< "fs_file::list_dir(): pattern = " << pattern << std::endl)

		WIN32_FIND_DATA wfd;
		HANDLE h = FindFirstFile(pattern.c_str(), &wfd);
		if (h == INVALID_HANDLE_VALUE) {
			throw std::system_error(GetLastError(), std::generic_category(), "FindFirstFile() failed");
		}

		//create Find Closer to automatically call FindClose on exit from the function in case of exceptions etc...
		{
			utki::ScopeExit scopeExit([h]() {
				FindClose(h);
			});

			do {
				std::string s(wfd.cFileName);
				ASSERT(s.size() > 0)

				//do not add ./ and ../ directories, we are not interested in them
				if (s == "." || s == "..") {
					continue;
				}

				if (((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) && s[s.size() - 1] != '/') {
					s += '/';
				}
				files.push_back(s);

				if (files.size() == maxEntries) {
					break;
				}
			} while (FindNextFile(h, &wfd) != 0);

			if (GetLastError() != ERROR_SUCCESS && GetLastError() != ERROR_NO_MORE_FILES) {
				throw papki::exception("list_dir(): find next file failed");
			}
		}
	}
#elif M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX
	{
		DIR *pdir = opendir(this->path().c_str());

		if(!pdir){
			std::stringstream ss;
			ss << "fs_file::list_dir(): opendir() failure, error code = " << strerror(errno);
			throw std::system_error(errno, std::generic_category(), ss.str());
		}

		//create DirentCloser to automatically call closedir on exit from the function in case of exceptions etc...
		struct DirCloser{
			DIR *pdir;

			DirCloser(DIR *pDirToClose) :
					pdir(pDirToClose)
			{}

			~DirCloser(){
				int ret;
				do{
					ret = closedir(this->pdir);
					ASSERT_INFO(ret == 0 || errno == EINTR, "fs_file::list_dir(): closedir() failed: " << strerror(errno))
				}while(ret != 0 && errno == EINTR);
			}
		} dirCloser(pdir);

		errno = 0;//clear errno
		while(dirent *pent = readdir(pdir)){
			std::string s(pent->d_name);
			if(s == "." || s == "..")
				continue;//do not add ./ and ../ directories, we are not interested in them

			struct stat fileStats;
			//TRACE(<< s << std::endl)
			if(stat((this->path() + s).c_str(), &fileStats) < 0){
				std::stringstream ss;
				ss << "fs_file::list_dir(): stat() failure, error code = " << strerror(errno);
				throw std::system_error(errno, std::system_category(), ss.str());
			}

			if(fileStats.st_mode & S_IFDIR)//if this entry is a directory append '/' symbol to its end
				s += "/";

			files.push_back(s);

			if(files.size() == maxEntries){
				break;
			}
		}//~while()

		//check if we exited the while() loop because of readdir() failed
		if(errno != 0){
			std::stringstream ss;
			ss << "fs_file::list_dir(): readdir() failure, error code = " << strerror(errno);
			throw std::system_error(errno, std::system_category(), ss.str());
		}
	}

#else

#	error "fs_file::list_dir(): version is not implemented yet for this os"

#endif
	return files;
}


std::unique_ptr<File> fs_file::spawn(){
	return utki::make_unique<fs_file>();
}
