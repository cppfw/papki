/**
 * @file File wrapper allowing to set root path.
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include "File.hpp"

namespace papki{

//TODO: doxygen
class RootDirFile : public File{
	std::unique_ptr<File> baseFile;
	std::string rootDir;
	
public:
	/**
	 * @param baseFile - a File to wrap.
	 * @param rootDir - path to the root directory to set. It should have trailing '/' character.
	 */
	RootDirFile(std::unique_ptr<File> baseFile, const std::string& rootDir) :
			baseFile(std::move(baseFile)),
			rootDir(rootDir)
	{
		if(!this->baseFile){
			throw File::Exc("RootDirFile(): passed in base file pointer is null");
		}
		this->File::SetPathInternal(this->baseFile->Path());
		this->baseFile->SetPath(this->rootDir + this->Path());
	}
	
	static std::unique_ptr<RootDirFile> New(std::unique_ptr<File> baseFile, const std::string& rootDir){
		return std::unique_ptr<RootDirFile>(new RootDirFile(std::move(baseFile), rootDir));
	}
	
	static std::unique_ptr<const RootDirFile> NewConst(std::unique_ptr<const File> baseFile, const std::string& rootDir){
		return std::unique_ptr<const RootDirFile>(new RootDirFile(std::unique_ptr<File>(const_cast<File*>(baseFile.release())), rootDir));
	}
	
	RootDirFile(const RootDirFile&) = delete;
	RootDirFile& operator=(const RootDirFile&) = delete;
	
private:
	void SetPathInternal(const std::string& pathName)const override{
		this->File::SetPathInternal(pathName);
		this->baseFile->SetPath(this->rootDir + pathName);
	}
	
	void OpenInternal(E_Mode mode)override{
		this->baseFile->Open(mode);
	}
	
	void CloseInternal()const noexcept override{
		this->baseFile->Close();
	}
	
	std::vector<std::string> ListDirContents(size_t maxEntries = 0)const override{
		return this->baseFile->ListDirContents(maxEntries);
	}
	
	size_t ReadInternal(utki::Buf<std::uint8_t> buf)const override{
		return this->baseFile->Read(buf);
	}
	
	size_t WriteInternal(utki::Buf<const std::uint8_t> buf)override{
		return this->baseFile->Write(buf);
	}
	
	size_t SeekForwardInternal(size_t numBytesToSeek)const override{
		return this->baseFile->SeekForward(numBytesToSeek);
	}
	
	size_t SeekBackwardInternal(size_t numBytesToSeek)const override{
		return this->baseFile->SeekBackward(numBytesToSeek);
	}
	
	void RewindInternal()const override{
		this->baseFile->Rewind();
	}
	
	void MakeDir()override{
		this->baseFile->MakeDir();
	}
	
	bool Exists()const override{
		return this->baseFile->Exists();
	}
	
	std::unique_ptr<File> Spawn()override{
		return New(this->baseFile->Spawn(), this->rootDir);
	}
};

}
