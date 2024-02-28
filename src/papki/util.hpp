/*
The MIT License (MIT)

Copyright (c) 2015-2024 Ivan Gagis <igagis@gmail.com>

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

#include <string>

namespace papki {

/**
 * @brief Check if path represents a directory.
 * @param path_name - path name to check.
 * @return true if path name ends with forward slash character '/'.
 * @return false otherwise.
 */
bool is_dir(std::string_view path_name);

/**
 * @brief Omit directories from path string.
 * For example, if path is '/home/user/some.file.txt' then the return value
 * will be 'some.file.txt'.
 * @param path_name - path string to omit directories from.
 * @return filename.
 */
std::string not_dir(std::string_view path_name);

/**
 * @brief Get directory part of the path.
 * Example: if path is '/home/user/some.file.txt' then the return value
 * will be '/home/user/'.
 * @param path_name - path string to get directory part from.
 * @return String representation of directory part of the path.
 */
std::string dir(std::string_view path_name);

/**
 * @brief Get file suffix.
 * Returns a string containing the tail part of the file path, everything that
 * goes after the last dot character ('.') in the file path string.
 * I.e. if the file path is '/home/user/some.file.txt' then the return value
 * will be 'txt'.
 * Note, that on *nix systems if the file name starts with a dot then this file
 * is treated as hidden, in that case it is thought that the file has no suffix.
 * I.e., for example , if the file path is '/home/user/.myfile' then the file
 * has no suffix and this function will return an empty string. Although, if the
 * file path is '/home/user/.myfile.txt' then the file does have a suffix and
 * the function will return 'txt'.
 * @param path_name - path string to get the suffix from.
 * @return String representing file suffix.
 */
std::string suffix(std::string_view path_name);

/**
 * @brief Get file name without suffix.
 * Returns a string containing the file path without suffix, everything that
 * goes after the last dot character ('.') is trimmed, including the dot
 * character. I.e. if the file path is '/home/user/some.file.txt' then the
 * return value will be '/home/user/some.file'. Note, that on *nix systems if
 * the file name starts with a dot then this file is treated as hidden, in that
 * case it is thought that the file has no suffix. I.e., for example , if the
 * file path is '/home/user/.myfile' then the file has no suffix and this
 * function will return same string, i.e. '/home/user/.myfile'. Although, if the
 * file path is '/home/user/.myfile.txt' then the file does have a suffix and
 * the function will return '/home/user/.myfile'.
 * @param path_name - path string to trim the suffix from.
 * @return String representing file name without suffix.
 */
std::string not_suffix(std::string_view path_name);

/**
 * @brief Append trailing slash if needed.
 * @param path - path to append trailing slash
 * @return Path name with appended trailing slash.
 */
std::string as_dir(std::string_view path);

std::string_view as_file(std::string_view path);

} // namespace papki
