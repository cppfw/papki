#include <utki/debug.hpp>
#include "../../src/papki/zip_file.hpp"
#include "../../src/papki/fs_file.hpp"

#include "tests.hpp"

namespace test_papki_zip_file{
void run(){
	// list directory contents
	{
		papki::zip_file zip_f(std::make_unique<papki::fs_file>("test.zip"));
		ASSERT_ALWAYS(!zip_f.is_dir());
		ASSERT_ALWAYS(!zip_f.is_open())

		zip_f.set_path("./");
		ASSERT_ALWAYS(zip_f.is_dir());
		ASSERT_ALWAYS(!zip_f.is_open())

		{
			auto contents = zip_f.list_dir();

			ASSERT_ALWAYS(contents.size() == 3)

			// for(auto& f : contents){
			// 	TRACE_ALWAYS(<< f << std::endl)
			// }
			
			ASSERT_ALWAYS(contents[0] == "test1.txt")
			ASSERT_ALWAYS(contents[1] == "dir1/")
			ASSERT_ALWAYS(contents[2] == "dir2/")
		}

		zip_f.set_path("dir1/");
		ASSERT_ALWAYS(zip_f.is_dir());
		ASSERT_ALWAYS(!zip_f.is_open())
		{
			auto contents = zip_f.list_dir();

			ASSERT_ALWAYS(contents.size() == 1)

			// for(auto& f : contents){
			// 	TRACE_ALWAYS(<< f << std::endl)
			// }
			
			ASSERT_ALWAYS(contents[0] == "test2.txt")
		}

		zip_f.set_path("dir2/");
		ASSERT_ALWAYS(zip_f.is_dir());
		ASSERT_ALWAYS(!zip_f.is_open())
		{
			auto contents = zip_f.list_dir();

			ASSERT_ALWAYS(contents.size() == 1)

			// for(auto& f : contents){
			// 	TRACE_ALWAYS(<< f << std::endl)
			// }
			
			ASSERT_ALWAYS(contents[0] == "test3.txt")
		}
	}

	// reading file
	{
		papki::zip_file zip_f(std::make_unique<papki::fs_file>("test.zip"), "dir1/test2.txt");
		ASSERT_ALWAYS(!zip_f.is_dir())
		ASSERT_ALWAYS(!zip_f.is_open())

		auto contents = zip_f.load();

		ASSERT_ALWAYS(!contents.empty())

		std::string str(reinterpret_cast<char*>(contents.data()), contents.size());

		// TRACE(<< "f = " << str << std::endl)

		ASSERT_INFO_ALWAYS(str == "test file #2.\n", "str = " << str)
	}
}
}
