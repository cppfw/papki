#pragma once

#include <string>

namespace papki{

/**
 * @brief Check if path represents a directory.
 * @param path_name - path name to check.
 * @return true if path name ends with forward slash character '/'.
 * @return false otherwise.
 */
bool is_dir(const std::string& path_name);

/**
 * @brief Omit directories from path string.
 * For example, if path is '/home/user/some.file.txt' then the return value
 * will be 'some.file.txt'.
 * @param path_name - path string to omit directories from.
 * @return filename.
 */
std::string not_dir(const std::string& path_name);

/**
 * @brief Get directory part of the path.
 * Example: if path is '/home/user/some.file.txt' then the return value
 * will be '/home/user/'.
 * @param path_name - path string to get directory part from.
 * @return String representation of directory part of the path.
 */
std::string dir(const std::string& path_name);

/**
 * @brief Get file suffix.
 * Returns a string containing the tail part of the file path, everything that
 * goes after the last dot character ('.') in the file path string.
 * I.e. if the file path is '/home/user/some.file.txt' then the return value
 * will be 'txt'.
 * Note, that on *nix systems if the file name starts with a dot then this file is treated as hidden,
 * in that case it is thought that the file has no suffix. I.e., for example
 * , if the file path is '/home/user/.myfile' then the file has no suffix and this function
 * will return an empty string. Although, if the file path is '/home/user/.myfile.txt' then the file
 * does have a suffix and the function will return 'txt'.
 * @param path_name - path string to get the suffix from.
 * @return String representing file suffix.
 */
std::string suffix(const std::string& path_name);

}
