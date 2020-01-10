#pragma once

#include <string>

namespace papki{

/**
 * @brief Check if path represents a directory.
 * @param pathname - pathname to check.
 * @return true if pathname ends with forward slash character '/'.
 * @return false otherwise.
 */
bool is_dir(const std::string& pathname);

}
