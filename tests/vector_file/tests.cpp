#include <utki/debug.hpp>
#include "../../src/papki/vector_file.hpp"

#include "tests.hpp"




namespace TestBasicMemoryFile{
void Run(){
	papki::vector_file f;
	ASSERT_ALWAYS(!f.is_dir())
	ASSERT_ALWAYS(!f.is_open())
	ASSERT_ALWAYS(f.size() == 0)

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
		
		ASSERT_ALWAYS(b[0] == 1)
		ASSERT_ALWAYS(b[1] == 2)
		ASSERT_ALWAYS(b[2] == 3)
		ASSERT_ALWAYS(b[3] == 4)
	}
}
}
