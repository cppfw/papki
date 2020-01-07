/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <utki/config.hpp>

#include "file.hpp"


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
	utki::span<std::uint8_t> data;
	mutable decltype(data)::iterator ptr;
	
public:
	/**
	 * @brief Constructor.
	 * @param data - reference to a memory buffer holding data of the file.
	 *               The reference to the buffer is saved in the BufferFile object,
	 *               but ownership of the buffer is not taken. Thus, the buffer should remain alive during lifetime of this BufferFile object.
	 */
	//NOTE: ownership of the buffer is not taken, buffer must remain alive during this object's lifetime.
	BufferFile(utki::span<std::uint8_t> data) :
			data(data)
	{}
	
	~BufferFile()noexcept override{}

	virtual std::unique_ptr<File> spawn()override{
		throw utki::exception("BufferFile::Spawn(): spawning is not supported.");
	}

protected:

	void open_internal(mode mode)override;
	
	void close_internal()const noexcept override{}
	
	size_t read_internal(utki::span<std::uint8_t> buf)const override;

	size_t write_internal(const utki::span<std::uint8_t> buf)override;
	
	size_t seek_forward_internal(size_t numBytesToSeek)const override;
	
	size_t seek_backward_internal(size_t numBytesToSeek)const override;
	
	void rewind_internal()const override;
};

}//~namespace
