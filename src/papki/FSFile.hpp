#pragma once

#include <cstdio>
#include <memory>

#include <utki/debug.hpp>
#include <utki/config.hpp>

#include "File.hpp"



namespace papki{



/**
 * @brief Native OS file system implementation of File interface.
 * Implementation of a ting::File interface for native file system of the OS.
 */
class FSFile : public File{
	mutable FILE* handle = nullptr;

protected:
	void openInternal(E_Mode mode)override;

	void closeInternal()const noexcept override;

	size_t readInternal(utki::Buf<std::uint8_t> buf)const override;

	size_t writeInternal(const utki::Buf<std::uint8_t> buf)override;

	//NOTE: use default implementation of SeekForward() because of the problems with
	//      fseek() as it can set file pointer beyond the end of file.
	
	size_t seekBackwardInternal(size_t numBytesToSeek)const override;
	
	void rewindInternal()const override;
	
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
		this->close();
	}	
	
	bool exists()const override;
	
	void makeDir()override;

	/**
	 * @brief Get user home directory.
	 * Returns an absolute path to the current user's home directory.
	 * On *nix systems it will be something like "/home/user/".
     * @return Absolute path to the user's home directory.
     */
	static std::string getHomeDir();



	virtual std::vector<std::string> listDirContents(size_t maxEntries = 0)const override;
	
	virtual std::unique_ptr<File> spawn()override;
};



}//~namespace
