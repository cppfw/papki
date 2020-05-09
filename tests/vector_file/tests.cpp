#include <utki/debug.hpp>
#include "../../src/papki/MemoryFile.hpp"

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
		
		papki::File::Guard fileGuard(f, papki::File::mode::create);
		
		f.write(b);
	}
	
	{
		std::array<uint8_t, 4> b;
		
		papki::File::Guard fileGuard(f, papki::File::mode::read);
		
		f.read(utki::make_span(b));
		
		ASSERT_ALWAYS(b[0] == 1)
		ASSERT_ALWAYS(b[1] == 2)
		ASSERT_ALWAYS(b[2] == 3)
		ASSERT_ALWAYS(b[3] == 4)
	}
}
}
