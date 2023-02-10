#pragma once

#include <utki/debug.hpp>

#include "tests.hpp"

inline void test_papki_fs_file(){
	test_seek_forward::run();
	test_list_dir_contents::run();
	test_home_dir::run();
	test_load_whole_file_to_memory::run();
}
