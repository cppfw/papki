#include <utki/debug.hpp>
#include <utki/types.hpp>

#include "../../src/papki/fs_file.hpp"
#include "../../src/papki/root_dir.hpp"
#include "../../src/papki/util.hpp"

#include "tests.hpp"


namespace TestSeekForward{
void Run(){
	papki::fs_file f("test.file.txt");
	utki::assert(!f.is_dir(), SL);
	utki::assert(!f.is_open(), SL);
	
	for(unsigned numToSeek = 0; numToSeek < 0x1000; numToSeek += (0x1000 / 4)){
		std::array<uint8_t, 1> testByte;
		{
			std::vector<uint8_t> buf(numToSeek);
			
			papki::file::guard fileGuard(f, papki::file::mode::read);
			
			auto res = f.read(utki::make_span(buf));
			utki::assert(res == buf.size(), SL);
			
			res = f.read(utki::make_span(testByte));
			utki::assert(res == testByte.size(), SL);
			
//			TRACE_ALWAYS(<< "testByte = " << unsigned(testByte[0]) << std::endl)
		}
		
		{
			papki::file::guard fileGuard(f, papki::file::mode::read);

			f.file::seek_forward(numToSeek);

			std::array<uint8_t, 1> buf;

			auto res = f.read(utki::make_span(buf));
			utki::assert(res == 1, SL);

			utki::assert(buf[0] == testByte[0], SL);
		}

		{
			papki::file::guard fileGuard(f, papki::file::mode::read);

			f.seek_forward(numToSeek);

			std::array<uint8_t, 1> buf;

			auto res = f.read(utki::make_span(buf));
			utki::assert(res == 1, SL);

			utki::assert(buf[0] == testByte[0], SL);
		}
	}
}
}



namespace TestListDirContents{
void Run(){
	papki::fs_file curDir("./");
	papki::file& f = curDir;
	
	std::vector<std::string> r = f.list_dir();
	utki::assert(r.size() >= 3, SL);
//	TRACE_ALWAYS(<< "list = " << r << std::endl)
	
	std::vector<std::string> r1 = f.list_dir(1);
	utki::assert(r1.size() == 1, SL);
	utki::assert(r[0] == r1[0], SL);
	
	std::vector<std::string> r2 = f.list_dir(2);
	utki::assert(r2.size() == 2, SL);
	utki::assert(r[0] == r2[0], SL);
	utki::assert(r[1] == r2[1], SL);
}
}



namespace TestHomeDir{
void Run(){
	std::string hd = papki::fs_file::get_home_dir();
	
	utki::assert(hd.size() != 0, SL); // There is always a trailing '/' character, so make sure there is something else besides that.
	utki::assert(papki::is_dir(hd), SL);
	
	utki::log([&](auto&o){o << "\tHome dir = " << hd << std::endl;});
}
}



namespace TestLoadWholeFileToMemory{
void Run(){
	papki::root_dir f(utki::make_unique<papki::fs_file>(), "");
	f.set_path("test.file.txt");
	utki::assert(!f.is_dir(), SL);
	utki::assert(!f.is_open(), SL);
	
	{
		std::vector<uint8_t> r = f.load();
		utki::assert(r.size() == 66874, SL);
	}
	
	{
		std::vector<uint8_t> r = f.load(66874);
		utki::assert(r.size() == 66874, SL);
	}
	
	{
		std::vector<uint8_t> r = f.load(4096);
		utki::assert(r.size() == 4096, SL);
	}
	
	{
		std::vector<uint8_t> r = f.load(35);
		utki::assert(r.size() == 35, SL);
	}
	
	{
		std::vector<uint8_t> r = f.load(1000000);
		utki::assert(r.size() == 66874, SL);
	}
}
}
