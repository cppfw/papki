/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <utki/config.hpp>

#include "File.hpp"


namespace papki{

 
/**
 * @brief Memory buffer file.
 * A memory buffer represented as a File.
 * It supports reading, writing, seeking forward and backwards, rewinding.
 * The size of the file remains constant and is equal to the size of the memory
 * buffer used for storing the file data.
 */
class BufferFile : public File{
	
private:
	BufferFile(const BufferFile&) = delete;
	BufferFile(BufferFile&&) = delete;
	BufferFile& operator=(const BufferFile&) = delete;
	BufferFile& operator=(BufferFile&&) = delete;
	
private:
	utki::Buf<std::uint8_t> data;
	mutable decltype(data)::iterator ptr;
	
public:
	/**
	 * @brief Constructor.
	 * @param data - reference to a memory buffer holding data of the file.
	 *               The reference to the buffer is saved in the BufferFile object,
	 *               but ownership of the buffer is not taken. Thus, the buffer should remain alive during lifetime of this BufferFile object.
	 */
	//NOTE: ownership of the buffer is not taken, buffer must remain alive during this object's lifetime.
	BufferFile(utki::Buf<std::uint8_t> data) :
			data(data)
	{}
	
	~BufferFile()noexcept override{}

	virtual std::unique_ptr<File> spawn()override{
		throw papki::Exc("BufferFile::Spawn(): spawning is not supported.");
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
