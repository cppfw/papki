#include <utki/debug.hpp>
#include <utki/types.hpp>

#include "../../src/papki/fs_file.hpp"
#include "../../src/papki/root_dir.hpp"
#include "../../src/papki/util.hpp"

#include "tests.hpp"


namespace TestSeekForward{
void Run(){
	papki::fs_file f("test.file.txt");
	ASSERT_ALWAYS(!f.isDir())
	ASSERT_ALWAYS(!f.isOpened())
	
	for(unsigned numToSeek = 0; numToSeek < 0x1000; numToSeek += (0x1000 / 4)){
		std::array<uint8_t, 1> testByte;
		{
			std::vector<uint8_t> buf(numToSeek);
			
			papki::File::Guard fileGuard(f, papki::File::mode::read);
			
			auto res = f.read(utki::make_span(buf));
			ASSERT_ALWAYS(res == buf.size())
			
			res = f.read(utki::make_span(testByte));
			ASSERT_ALWAYS(res == testByte.size())
			
//			TRACE_ALWAYS(<< "testByte = " << unsigned(testByte[0]) << std::endl)
		}
		
		{
			papki::File::Guard fileGuard(f, papki::File::mode::read);

			f.file::seekForward(numToSeek);

			std::array<uint8_t, 1> buf;

			auto res = f.read(utki::make_span(buf));
			ASSERT_ALWAYS(res == 1)

			ASSERT_ALWAYS(buf[0] == testByte[0])
		}

		{
			papki::File::Guard fileGuard(f, papki::File::mode::read);

			f.seekForward(numToSeek);

			std::array<uint8_t, 1> buf;

			auto res = f.read(utki::make_span(buf));
			ASSERT_ALWAYS(res == 1)

			ASSERT_ALWAYS(buf[0] == testByte[0])
		}
	}//~for
}
}



namespace TestListDirContents{
void Run(){
	papki::fs_file curDir("./");
	papki::File& f = curDir;
	
	std::vector<std::string> r = f.list_dir();
	ASSERT_ALWAYS(r.size() >= 3)
//	TRACE_ALWAYS(<< "list = " << r << std::endl)
	
	std::vector<std::string> r1 = f.list_dir(1);
	ASSERT_ALWAYS(r1.size() == 1)
	ASSERT_ALWAYS(r[0] == r1[0])
	
	std::vector<std::string> r2 = f.list_dir(2);
	ASSERT_ALWAYS(r2.size() == 2)
	ASSERT_ALWAYS(r[0] == r2[0])
	ASSERT_ALWAYS(r[1] == r2[1])
}
}



namespace TestHomeDir{
void Run(){
	std::string hd = papki::fs_file::getHomeDir();
	
	ASSERT_ALWAYS(hd.size() != 0) // There is always a trailing '/' character, so make sure there is something else besides that.
	ASSERT_ALWAYS(papki::is_dir(hd))
	
	TRACE_ALWAYS(<< "\tHome dir = " << hd << std::endl)
}
}



namespace TestLoadWholeFileToMemory{
void Run(){
	papki::root_dir f(utki::make_unique<papki::fs_file>(), "");
	f.setPath("test.file.txt");
	ASSERT_ALWAYS(!f.isDir())
	ASSERT_ALWAYS(!f.isOpened())
	
	{
		std::vector<uint8_t> r = f.loadWholeFileIntoMemory();
		ASSERT_ALWAYS(r.size() == 66874)
	}
	
	{
		std::vector<uint8_t> r = f.loadWholeFileIntoMemory(66874);
		ASSERT_ALWAYS(r.size() == 66874)
	}
	
	{
		std::vector<uint8_t> r = f.loadWholeFileIntoMemory(4096);
		ASSERT_ALWAYS(r.size() == 4096)
	}
	
	{
		std::vector<uint8_t> r = f.loadWholeFileIntoMemory(35);
		ASSERT_ALWAYS(r.size() == 35)
	}
	
	{
		std::vector<uint8_t> r = f.loadWholeFileIntoMemory(1000000);
		ASSERT_ALWAYS(r.size() == 66874)
	}
}
}
