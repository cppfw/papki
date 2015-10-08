/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <utki/config.hpp>

#include "File.hpp"

#include <vector>



namespace papki{

/**
 * @brief Memory file.
 * Class representing a file stored in memory. Supports reading, writing, seeking backwards and forward, rewinding.
 */
class MemoryFile : public File{
	
	MemoryFile(const MemoryFile&) = delete;
	MemoryFile(MemoryFile&&) = delete;
	MemoryFile& operator=(const MemoryFile&) = delete;
	MemoryFile& operator=(MemoryFile&&) = delete;
	
private:
	std::vector<std::uint8_t> data;
	mutable size_t idx;
	
public:
	/**
	 * @brief Constructor.
	 * Creates empty memory file.
     */
	MemoryFile(){}
	
	virtual ~MemoryFile()noexcept{}

	/**
	 * @brief Current file size.
     * @return current size of the file.
     */
	size_t size(){
		return this->data.size();
	}
	

	virtual std::unique_ptr<File> spawn()override{
		return utki::makeUnique<MemoryFile>();
	}

	
	/**
	 * @brief Clear the data of the file.
	 * After this operation the file becomes empty.
     * @return Data previously held by this file.
     */
	decltype(data) resetData(){
		if(this->isOpened()){
			throw IllegalStateExc("MemoryFile::ResetData(): could not reset data while file is opened");
		}
		return std::move(this->data);
	}
	
protected:
	void openInternal(E_Mode mode)override;
	
	void closeInternal()const noexcept override{}
	
	size_t readInternal(utki::Buf<std::uint8_t> buf)const override;
	
	size_t writeInternal(const utki::Buf<std::uint8_t> buf)override;
	
	size_t seekForwardInternal(size_t numBytesToSeek)const override;
	
	size_t seekBackwardInternal(size_t numBytesToSeek)const override;
	
	void rewindInternal()const override;
};

}//~namespace
