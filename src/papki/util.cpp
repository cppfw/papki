/*
The MIT License (MIT)

Copyright (c) 2015-2025 Ivan Gagis <igagis@gmail.com>

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

#include "util.hpp"

#include <sstream>
#include <string_view>

#include <utki/debug.hpp>
#include <utki/string.hpp>

using namespace std::string_literals;

using namespace papki;

bool papki::is_dir(std::string_view path_name)
{
	return path_name.length() != 0 && path_name.back() == '/';
}

std::string papki::not_dir(std::string_view path_name)
{
	size_t slash_pos = path_name.rfind('/');
	if (slash_pos == std::string::npos) { // no slash found
		return std::string(path_name);
	}

	ASSERT(slash_pos > 0)
	ASSERT(path_name.size() > 0)
	ASSERT(path_name.size() >= slash_pos + 1)

	return std::string(path_name.substr(slash_pos + 1));
}

std::string papki::dir(std::string_view path_name)
{
	size_t slash_pos = path_name.rfind('/');
	if (slash_pos == std::string::npos) { // no slash found
		return {};
	}

	ASSERT(slash_pos > 0)
	ASSERT(path_name.size() > 0)
	ASSERT(path_name.size() >= slash_pos + 1)

	return std::string(path_name.substr(0, slash_pos + 1));
}

std::string papki::suffix(std::string_view path_name)
{
	size_t dot_pos = path_name.rfind('.');
	if (dot_pos == std::string::npos || dot_pos == 0) { // NOTE: dot_pos is 0 for hidden files in *nix systems
		return {};
	} else {
		ASSERT(dot_pos > 0)
		ASSERT(path_name.size() > 0)
		ASSERT(path_name.size() >= dot_pos + 1)

		// check for hidden file on *nix systems
		if (path_name[dot_pos - 1] == '/') {
			return {};
		}

		return std::string(path_name.substr(dot_pos + 1));
	}
	ASSERT(false)
}

std::string papki::not_suffix(std::string_view path_name)
{
	size_t dot_pos = path_name.rfind('.');
	if (dot_pos == std::string::npos || dot_pos == 0) { // NOTE: dot_pos is 0 for hidden files in *nix systems
		return std::string(path_name);
	} else {
		ASSERT(dot_pos > 0)
		ASSERT(path_name.size() > 0)
		ASSERT(path_name.size() >= dot_pos + 1)

		// check for hidden file on *nix systems
		if (path_name[dot_pos - 1] == '/') {
			return std::string(path_name);
		}

		return std::string(path_name.substr(0, dot_pos));
	}
	ASSERT(false)
}

std::string papki::as_dir(std::string_view path)
{
	if (path.empty()) {
		return "./"s;
	}

	if (path.back() == '/') {
		return std::string(path);
	}

	return utki::cat(path, '/');
}

std::string_view papki::as_file(std::string_view path)
{
	if (path.empty()) {
		return path;
	}
	if (path.back() == '/') {
		return path.substr(0, path.size() - 1);
	}
	return path;
}
