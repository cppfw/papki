#include "main.hpp"

#include "../../src/papki/fs_file.hpp"

#ifdef assert
#	undef assert
#endif

// NOLINTNEXTLINE(bugprone-exception-escape, "we want uncaught exceptions to fail the tests")
int main(int argc, char *argv[]){
	test_papki_fs_file();

	// test file size
	{
		papki::fs_file f("test.file.txt");

		auto size = f.size();

		utki::assert(size == 66874, [&](auto&o){o << "size = " << size;}, SL);
	}

	return 0;
}
