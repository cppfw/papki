#include <utki/debug.hpp>
#include "../../src/papki/zip_file.hpp"
#include "../../src/papki/fs_file.hpp"

#include "tests.hpp"

namespace test_papki_zip_file{
void run(){
	// list directory contents
	{
		papki::zip_file zip_f(std::make_unique<papki::fs_file>("test.zip"));
		utki::assert(!zip_f.is_dir(), SL);
		utki::assert(!zip_f.is_open(), SL);

		zip_f.set_path("./");
		utki::assert(zip_f.is_dir(), SL);
		utki::assert(!zip_f.is_open(), SL);

		{
			auto contents = zip_f.list_dir();

			utki::assert(contents.size() == 3, SL);

			// for(auto& f : contents){
			// 	TRACE_ALWAYS(<< f << std::endl)
			// }
			
			utki::assert(contents[0] == "test1.txt", SL);
			utki::assert(contents[1] == "dir1/", SL);
			utki::assert(contents[2] == "dir2/", SL);
		}

		zip_f.set_path("dir1/");
		utki::assert(zip_f.is_dir(), SL);
		utki::assert(!zip_f.is_open(), SL);
		{
			auto contents = zip_f.list_dir();

			utki::assert(contents.size() == 1, SL);

			// for(auto& f : contents){
			// 	TRACE_ALWAYS(<< f << std::endl)
			// }
			
			utki::assert(contents[0] == "test2.txt", SL);
		}

		zip_f.set_path("dir2/");
		utki::assert(zip_f.is_dir(), SL);
		utki::assert(!zip_f.is_open(), SL);
		{
			auto contents = zip_f.list_dir();

			utki::assert(contents.size() == 1, SL);

			// for(auto& f : contents){
			// 	TRACE_ALWAYS(<< f << std::endl)
			// }
			
			utki::assert(contents[0] == "test3.txt", SL);
		}
	}

	// reading file
	{
		papki::zip_file zip_f(std::make_unique<papki::fs_file>("test.zip"), "dir1/test2.txt");
		utki::assert(!zip_f.is_dir(), SL);
		utki::assert(!zip_f.is_open(), SL);

		auto contents = zip_f.load();

		utki::assert(!contents.empty(), SL);

		std::string str(reinterpret_cast<char*>(contents.data()), contents.size());

		// TRACE(<< "f = " << str << std::endl)

		ASSERT_INFO_ALWAYS(str == "test file #2.\n", "str = " << str)
	}
}
}
