#include <utki/debug.hpp>
#include <utki/types.hpp>

#include "../../src/papki/FSFile.hpp"
#include "../../src/papki/RootDirFile.hpp"

#include "tests.hpp"





namespace TestSeekForward{
void Run(){
	papki::FSFile f("test.file.txt");
	ASSERT_ALWAYS(!f.isDir())
	ASSERT_ALWAYS(!f.isOpened())
	
	for(unsigned numToSeek = 0; numToSeek < 0x1000; numToSeek += (0x1000 / 4)){
		std::array<std::uint8_t, 1> testByte;
		{
			std::vector<std::uint8_t> buf(numToSeek);
			
			papki::File::Guard fileGuard(f, papki::File::E_Mode::READ);
			
			unsigned res = f.read(utki::wrapBuf(buf));
			ASSERT_ALWAYS(res == buf.size())
			
			res = f.read(utki::wrapBuf(testByte));
			ASSERT_ALWAYS(res == testByte.size())
			
//			TRACE_ALWAYS(<< "testByte = " << unsigned(testByte[0]) << std::endl)
		}
		
		{
			papki::File::Guard fileGuard(f, papki::File::E_Mode::READ);

			f.File::seekForward(numToSeek);

			std::array<std::uint8_t, 1> buf;

			unsigned res = f.read(utki::wrapBuf(buf));
			ASSERT_ALWAYS(res == 1)

			ASSERT_ALWAYS(buf[0] == testByte[0])
		}

		{
			papki::File::Guard fileGuard(f, papki::File::E_Mode::READ);

			f.seekForward(numToSeek);

			std::array<std::uint8_t, 1> buf;

			unsigned res = f.read(utki::wrapBuf(buf));
			ASSERT_ALWAYS(res == 1)

			ASSERT_ALWAYS(buf[0] == testByte[0])
		}
	}//~for
}
}//~namespace



namespace TestListDirContents{
void Run(){
	papki::FSFile curDir("./");
	papki::File& f = curDir;
	
	std::vector<std::string> r = f.listDirContents();
	ASSERT_ALWAYS(r.size() >= 3)
//	TRACE_ALWAYS(<< "list = " << r << std::endl)
	
	std::vector<std::string> r1 = f.listDirContents(1);
	ASSERT_ALWAYS(r1.size() == 1)
	ASSERT_ALWAYS(r[0] == r1[0])
	
	std::vector<std::string> r2 = f.listDirContents(2);
	ASSERT_ALWAYS(r2.size() == 2)
	ASSERT_ALWAYS(r[0] == r2[0])
	ASSERT_ALWAYS(r[1] == r2[1])
}
}//~namespace



namespace TestHomeDir{
void Run(){
	std::string hd = papki::FSFile::getHomeDir();
	
	ASSERT_ALWAYS(hd.size() > 1) //There is always a trailing '/' character, so make sure there is something else besides that.
	ASSERT_ALWAYS(hd[hd.size() - 1] == '/')
	
	TRACE_ALWAYS(<< "\tHome dir = " << hd << std::endl)
}
}//~namespace



namespace TestLoadWholeFileToMemory{
void Run(){
	papki::RootDirFile f(utki::makeUnique<papki::FSFile>(), "");
	f.setPath("test.file.txt");
	ASSERT_ALWAYS(!f.isDir())
	ASSERT_ALWAYS(!f.isOpened())
	
	{
		std::vector<std::uint8_t> r = f.loadWholeFileIntoMemory();
		ASSERT_ALWAYS(r.size() == 66874)
	}
	
	{
		std::vector<std::uint8_t> r = f.loadWholeFileIntoMemory(66874);
		ASSERT_ALWAYS(r.size() == 66874)
	}
	
	{
		std::vector<std::uint8_t> r = f.loadWholeFileIntoMemory(4096);
		ASSERT_ALWAYS(r.size() == 4096)
	}
	
	{
		std::vector<std::uint8_t> r = f.loadWholeFileIntoMemory(35);
		ASSERT_ALWAYS(r.size() == 35)
	}
	
	{
		std::vector<std::uint8_t> r = f.loadWholeFileIntoMemory(1000000);
		ASSERT_ALWAYS(r.size() == 66874)
	}
}
}//~namespace
