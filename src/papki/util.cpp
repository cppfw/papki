#include "util.hpp"

#include <utki/debug.hpp>

using namespace papki;

bool papki::is_dir(const std::string& pathname){
    return pathname.length() != 0 && pathname.back() == '/';
}

std::string papki::not_dir(const std::string& pathname){
    size_t slashPos = pathname.rfind('/');
	if(slashPos == std::string::npos){ // no slash found
		return pathname;
	}

	ASSERT(slashPos > 0)
	ASSERT(pathname.size() > 0)
	ASSERT(pathname.size() >= slashPos + 1)

	return std::string(pathname, slashPos + 1);
}

std::string papki::dir(const std::string& pathname){
    size_t slashPos = pathname.rfind('/');
	if(slashPos == std::string::npos){ // no slash found
		return std::string();
	}

	ASSERT(slashPos > 0)
	ASSERT(pathname.size() > 0)
	ASSERT(pathname.size() >= slashPos + 1)

	return std::string(pathname, 0, slashPos + 1);
}

std::string papki::suffix(const std::string& pathname){
    size_t dotPos = pathname.rfind('.');
	if(dotPos == std::string::npos || dotPos == 0){ // NOTE: dotPos is 0 for hidden files in *nix systems
		return std::string();
	}else{
		ASSERT(dotPos > 0)
		ASSERT(pathname.size() > 0)
		ASSERT(pathname.size() >= dotPos + 1)
		
		// Check for hidden file on *nix systems
		if(pathname[dotPos - 1] == '/'){
			return std::string();
		}
		
		return std::string(pathname, dotPos + 1, pathname.size() - (dotPos + 1));
	}
	ASSERT(false)
}
