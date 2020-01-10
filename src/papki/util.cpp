#include "util.hpp"


using namespace papki;

bool papki::is_dir(const std::string& pathname){
    return pathname.length() != 0 && pathname.back() == '/';
}
