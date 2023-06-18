/*
The MIT License (MIT)

Copyright (c) 2015-2023 Ivan Gagis <igagis@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/* ================ LICENSE END ================ */

#pragma once

#include <memory>

#include <utki/debug.hpp>

#include "file.hpp"

namespace papki {

// TODO: doxygen
class zip_file : public papki::file
{
	std::unique_ptr<papki::file> underlying_zip_file;

	void* handle = nullptr;

public:
	zip_file(std::unique_ptr<papki::file> underlying_zip_file, std::string_view path = std::string_view());

	zip_file(const zip_file&) = delete;
	zip_file& operator=(const zip_file&) = delete;

	zip_file(zip_file&&) = delete;
	zip_file& operator=(zip_file&&) = delete;

	~zip_file() noexcept override;

	void open_internal(papki::file::mode mode) override;
	void close_internal() const noexcept override;
	size_t read_internal(utki::span<uint8_t> buf) const override;
	bool exists() const override;
	std::vector<std::string> list_dir(size_t max_entries = 0) const override;

	std::unique_ptr<papki::file> spawn() override
	{
		std::unique_ptr<papki::file> zf = this->underlying_zip_file->spawn();
		zf->set_path(this->underlying_zip_file->path());
		return std::make_unique<zip_file>(std::move(zf));
	}
};

} // namespace papki
