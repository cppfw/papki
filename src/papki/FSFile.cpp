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

#include "FSFile.hpp"



using namespace papki;



void FSFile::openInternal(E_Mode mode){
	if(this->isDir()){
		throw papki::Exc("path refers to a directory, directories can't be opened");
	}
	
	const char* modeStr;
	switch(mode){
		case File::E_Mode::WRITE:
			modeStr = "r+b";
			break;
		case File::E_Mode::CREATE:
			modeStr = "w+b";
			break;
		case File::E_Mode::READ:
			modeStr = "rb";
			break;
		default:
			throw papki::Exc("unknown mode");
			break;
	}

#if M_COMPILER == M_COMPILER_MSVC
	if(fopen_s(&this->handle, this->path().c_str(), modeStr) != 0){
		this->handle = 0;
	}
#else
	this->handle = fopen(this->path().c_str(), modeStr);
#endif
	if(!this->handle){
		TRACE(<< "FSFile::Open(): Path() = " << this->path().c_str() << std::endl)
		std::stringstream ss;
		ss << "fopen(" << this->path().c_str() << ") failed";
		throw papki::Exc(ss.str());
	}
}



//override
void FSFile::closeInternal()const noexcept{
	ASSERT(this->handle)

	fclose(this->handle);
	this->handle = 0;
}



//override
size_t FSFile::readInternal(utki::Buf<std::uint8_t> buf)const{
	ASSERT(this->handle)
	size_t numBytesRead = fread(buf.begin(), 1, buf.size(), this->handle);
	if(numBytesRead != buf.size()){//something happened
		if(!feof(this->handle)){
			throw papki::Exc("fread() error");//if it is not an EndOfFile then it is error
		}
	}
	return numBytesRead;
}



//override
size_t FSFile::writeInternal(const utki::Buf<std::uint8_t> buf){
	ASSERT(this->handle)
	size_t bytesWritten = fwrite(buf.begin(), 1, buf.size(), this->handle);
	if(bytesWritten != buf.size()){//something bad has happened
		throw papki::Exc("fwrite error");
	}

	return bytesWritten;
}



size_t FSFile::seekBackwardInternal(size_t numBytesToSeek)const{
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
			throw papki::Exc("fseek() failed");
		}
		
		ASSERT(size_t(offset) < size_t(-1))
		ASSERT(numBytesLeft >= size_t(offset))
		
		numBytesLeft -= size_t(offset);
	}
	
	return numBytesToSeek;
}



//override
void FSFile::rewindInternal()const{
	if(!this->isOpened()){
		throw papki::IllegalStateExc("cannot rewind, file is not opened");
	}

	ASSERT(this->handle)
	if(fseek(this->handle, 0, SEEK_SET) != 0){
		throw papki::Exc("fseek() failed");
	}
}



//override
bool FSFile::exists()const{
	if(this->isOpened()){ //file is opened => it exists
		return true;
	}
	
	if(this->path().size() == 0){
		return false;
	}

	//if it is a directory, check directory existence
	if(this->path()[this->path().size() - 1] == '/'){
#if M_OS == M_OS_LINUX
		DIR *pdir = opendir(this->path().c_str());
		if(!pdir){
			return false;
		}else{
			closedir(pdir);
			return true;
		}
#else
		throw papki::Exc("Checking for directory existence is not supported");
#endif
	}else{
		return this->File::exists();
	}
}



//override
void FSFile::makeDir(){
	if(this->isOpened()){
		throw papki::IllegalStateExc("cannot make directory when file is opened");
	}

	if(this->path().size() == 0 || this->path()[this->path().size() - 1] != '/'){
		throw papki::Exc("invalid directory name");
	}

#if M_OS == M_OS_LINUX
//	TRACE(<< "creating directory = " << this->Path() << std::endl)
	umask(0);//clear umask for proper permissions of newly created directory
	if(mkdir(this->path().c_str(), 0777) != 0){
		throw papki::Exc("mkdir() failed");
	}
#else
	throw papki::Exc("creating directory is not supported");
#endif
}



std::string FSFile::getHomeDir() {
	std::string ret;

#if M_OS == M_OS_WINDOWS && M_COMPILER == M_COMPILER_MSVC
	{
		char* buf = nullptr;
		size_t size = 0;

		utki::ScopeExit scopeExit([&buf](){
			free(buf);
		});

		if (_dupenv_s(&buf, &size, "USERPROFILE") != 0) {
			throw papki::Exc("USERPROFILE  environment virable could not be read");
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
		throw papki::Exc("HOME environment variable does not exist");
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


std::vector<std::string> FSFile::listDirContents(size_t maxEntries)const{
	if(!this->isDir()){
		throw papki::Exc("FSFile::ListDirContents(): this is not a directory");
	}

	std::vector<std::string> files;

#if M_OS == M_OS_WINDOWS
	{
		std::string pattern = this->path();
		pattern += '*';

		TRACE(<< "FSFile::ListDirContents(): pattern = " << pattern << std::endl)

		WIN32_FIND_DATA wfd;
		HANDLE h = FindFirstFile(pattern.c_str(), &wfd);
		if (h == INVALID_HANDLE_VALUE) {
			throw papki::Exc("ListDirContents(): cannot find first file");
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
				throw papki::Exc("ListDirContents(): find next file failed");
			}
		}
	}
#elif M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX
	{
		DIR *pdir = opendir(this->path().c_str());

		if(!pdir){
			std::stringstream ss;
			ss << "FSFile::ListDirContents(): opendir() failure, error code = " << strerror(errno);
			throw papki::Exc(ss.str());
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
					ASSERT_INFO(ret == 0 || errno == EINTR, "FSFile::ListDirContents(): closedir() failed: " << strerror(errno))
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
				ss << "FSFile::ListDirContents(): stat() failure, error code = " << strerror(errno);
				throw papki::Exc(ss.str());
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
			ss << "FSFile::ListDirContents(): readdir() failure, error code = " << strerror(errno);
			throw papki::Exc(ss.str());
		}
	}

#else

#	error "FSFile::ListDirContents(): version is not implemented yet for this os"

#endif
	return files;
}


std::unique_ptr<File> FSFile::spawn(){
	return utki::makeUnique<FSFile>();
}
