#pragma once

#include <utki/debug.hpp>

#include "file.hpp"

#include <memory>

namespace papki{

// TODO: doxygen
class zip_file : public papki::file{
	std::unique_ptr<papki::file> underlying_zip_file;
	
	void* handle = nullptr;
public:
	zip_file(std::unique_ptr<papki::file> underlying_zip_file, const std::string& path = std::string());

	~zip_file()noexcept;

	void open_internal(papki::file::mode mode) override;
	void close_internal()const noexcept override;
	size_t read_internal(utki::span<uint8_t> buf)const override;
	bool exists() const override;
	std::vector<std::string> list_dir(size_t max_entries = 0)const override;
	
	std::unique_ptr<papki::file> spawn()override{
		std::unique_ptr<papki::file> zf = this->underlying_zip_file->spawn();
		zf->set_path(this->underlying_zip_file->path());
		return std::make_unique<zip_file>(std::move(zf));
	}
};

}
