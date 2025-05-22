#include <utki/debug.hpp>

#include "../../src/papki/fs_file.hpp"

int main(int /* argc */, const char** /* argv */){
	{
		std::vector<uint8_t> bytes = papki::fs_file("test_data.bin").load();

		utki::assert(bytes.size() == 0x4000, [&](auto&o){o << "bytes.size() = " << bytes.size();}, SL);
	}

	{
		std::vector<uint8_t> bytes = papki::fs_file("test_data1.bin").load();

		utki::assert(bytes.size() == 49179, [&](auto&o){o << "bytes.size() = " << bytes.size();}, SL);
	}

	return 0;
}
