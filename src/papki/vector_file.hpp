#pragma once

#include <utki/config.hpp>

#include "file.hpp"

#include <vector>



namespace papki{

/**
 * @brief Memory file.
 * Class representing a file stored in memory. Supports reading, writing, seeking backwards and forward, rewinding.
 */
class vector_file : public file{
	
	vector_file(const vector_file&) = delete;
	vector_file(vector_file&&) = delete;
	vector_file& operator=(const vector_file&) = delete;
	vector_file& operator=(vector_file&&) = delete;
	
private:
	std::vector<std::uint8_t> data;
	mutable size_t idx;
	
public:
	/**
	 * @brief Constructor.
	 * Creates empty memory file.
     */
	vector_file(){}
	
	virtual ~vector_file()noexcept{}

	/**
	 * @brief Current file size.
     * @return current size of the file.
     */
	uint64_t size()const override{
		return this->data.size();
	}
	

	virtual std::unique_ptr<file> spawn()override{
		return utki::make_unique<vector_file>();
	}

	
	/**
	 * @brief Clear the data of the file.
	 * After this operation the file becomes empty.
     * @return Data previously held by this file.
     */
	decltype(data) reset_data(){
		if(this->is_open()){
			throw utki::invalid_state("vector_file::reset_data(): could not reset data while file is opened");
		}
		return std::move(this->data);
	}

	//TODO: deprecated, remove.
	decltype(data) resetData(){
		return this->reset_data();
	}
	
protected:
	void open_internal(mode io_mode)override;
	
	void close_internal()const noexcept override{}
	
	size_t read_internal(utki::span<std::uint8_t> buf)const override;
	
	size_t write_internal(const utki::span<std::uint8_t> buf)override;
	
	size_t seek_forward_internal(size_t numBytesToSeek)const override;
	
	size_t seek_backward_internal(size_t numBytesToSeek)const override;
	
	void rewind_internal()const override;
};

}
