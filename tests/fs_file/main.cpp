#include "main.hpp"

#include "../../src/papki/fs_file.hpp"

int main(int argc, char *argv[]){
	TestTingFSFile();

	// test file size
	{
		papki::fs_file f("test.file.txt");

		auto size = f.size();

		ASSERT_INFO_ALWAYS(size == 66874, "size = " << size)
	}

	return 0;
}
