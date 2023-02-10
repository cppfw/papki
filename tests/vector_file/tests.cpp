#include <utki/debug.hpp>
#include "../../src/papki/vector_file.hpp"

#include "tests.hpp"




namespace TestBasicMemoryFile{
void run(){
	papki::vector_file f;
	utki::assert(!f.is_dir(), SL);
	utki::assert(!f.is_open(), SL);
	utki::assert(f.size() == 0, SL);

	{
		uint8_t buf[] = {1, 2, 3, 4};
		auto b = utki::make_span(buf, sizeof(buf));
		
		papki::file::guard fileGuard(f, papki::file::mode::create);
		
		f.write(b);
	}
	
	{
		std::array<uint8_t, 4> b;
		
		papki::file::guard fileGuard(f, papki::file::mode::read);
		
		f.read(utki::make_span(b));
		
		utki::assert(b[0] == 1, SL);
		utki::assert(b[1] == 2, SL);
		utki::assert(b[2] == 3, SL);
		utki::assert(b[3] == 4, SL);
	}
}
}
