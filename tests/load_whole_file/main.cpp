#include <utki/debug.hpp>

#include "../../src/papki/fs_file.hpp"


int main(int argc, char *argv[]){
	{
		std::vector<uint8_t> bytes = papki::fs_file("test_data.bin").load();

		ASSERT_INFO_ALWAYS(bytes.size() == 0x4000, "bytes.size() = " << bytes.size())
	}

	{
		std::vector<uint8_t> bytes = papki::fs_file("test_data1.bin").load();

		ASSERT_INFO_ALWAYS(bytes.size() == 49179, "bytes.size() = " << bytes.size())
	}

	return 0;
}
