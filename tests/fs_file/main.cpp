#include "main.hpp"

#include "../../src/papki/fs_file.hpp"

int main(int argc, char *argv[]){
	TestTingFSFile();

	// test file size
	{
		papki::fs_file f("test.file.txt");

		auto size = f.size();

		utki::assert(size == 66874, [&](auto&o){o << "size = " << size;}, SL);
	}

	return 0;
}
