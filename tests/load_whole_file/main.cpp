#include <utki/debug.hpp>

#include "../../src/papki/FSFile.hpp"


int main(int argc, char *argv[]){
	{
		std::vector<uint8_t> bytes = papki::FSFile("test_data.bin").loadWholeFileIntoMemory();

		ASSERT_INFO_ALWAYS(bytes.size() == 0x4000, "bytes.size() = " << bytes.size())
	}

	{
		std::vector<uint8_t> bytes = papki::FSFile("test_data1.bin").loadWholeFileIntoMemory();

		ASSERT_INFO_ALWAYS(bytes.size() == 49179, "bytes.size() = " << bytes.size())
	}

	return 0;
}
