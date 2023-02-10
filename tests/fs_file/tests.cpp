#include <utki/debug.hpp>
#include <utki/types.hpp>

#include "../../src/papki/fs_file.hpp"
#include "../../src/papki/root_dir.hpp"
#include "../../src/papki/util.hpp"

#include "tests.hpp"

#ifdef assert
#	undef assert
#endif

namespace test_seek_forward{
void run(){
	papki::fs_file f("test.file.txt");
	utki::assert(!f.is_dir(), SL);
	utki::assert(!f.is_open(), SL);
	
	for(unsigned num_to_seek = 0; num_to_seek < 0x1000; num_to_seek += (0x1000 / 4)){
		std::array<uint8_t, 1> test_byte;
		{
			std::vector<uint8_t> buf(num_to_seek);
			
			papki::file::guard file_guard(f, papki::file::mode::read);
			
			auto res = f.read(utki::make_span(buf));
			utki::assert(res == buf.size(), SL);
			
			res = f.read(utki::make_span(test_byte));
			utki::assert(res == test_byte.size(), SL);
		}
		
		{
			papki::file::guard file_guard(f, papki::file::mode::read);

			f.file::seek_forward(num_to_seek);

			std::array<uint8_t, 1> buf;

			auto res = f.read(utki::make_span(buf));
			utki::assert(res == 1, SL);

			utki::assert(buf[0] == test_byte[0], SL);
		}

		{
			papki::file::guard file_guard(f, papki::file::mode::read);

			f.seek_forward(num_to_seek);

			std::array<uint8_t, 1> buf;

			auto res = f.read(utki::make_span(buf));
			utki::assert(res == 1, SL);

			utki::assert(buf[0] == test_byte[0], SL);
		}
	}
}
}



namespace test_list_dir_contents{
void run(){
	papki::fs_file cur_dir("./");
	papki::file& f = cur_dir;
	
	std::vector<std::string> r = f.list_dir();
#ifdef DEBUG
	for(const auto& e : r){
		utki::log([&](auto& o){o << "e = " << e << '\n';});
	}
#endif
	utki::assert(r.size() >= 3, SL);
	
	std::vector<std::string> r1 = f.list_dir(1);
	utki::assert(r1.size() == 1, SL);
	utki::assert(r[0] == r1[0], SL);
	
	std::vector<std::string> r2 = f.list_dir(2);
	utki::assert(r2.size() == 2, SL);
	utki::assert(r[0] == r2[0], SL);
	utki::assert(r[1] == r2[1], SL);
}
}



namespace test_home_dir{
void run(){
	std::string hd = papki::fs_file::get_home_dir();
	
	utki::assert(hd.size() != 0, SL); // There is always a trailing '/' character, so make sure there is something else besides that.
	utki::assert(papki::is_dir(hd), SL);
	
	utki::log([&](auto&o){o << "\tHome dir = " << hd << std::endl;});
}
}



namespace test_load_whole_file_to_memory{
void run(){
	papki::root_dir f(std::make_unique<papki::fs_file>(), "");
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
