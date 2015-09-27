#include "../../src/ting/debug.hpp"
#include "../../src/ting/fs/MemoryFile.hpp"

#include "tests.hpp"



using namespace ting;



namespace TestBasicMemoryFile{
void Run(){
	ting::fs::MemoryFile f;
	ASSERT_ALWAYS(!f.IsDir())
	ASSERT_ALWAYS(!f.IsOpened())
	ASSERT_ALWAYS(f.Size() == 0)

	{
		std::uint8_t buf[] = {1, 2, 3, 4};
		ting::Buffer<std::uint8_t> b(buf, sizeof(buf));
		
		ting::fs::File::Guard fileGuard(f, ting::fs::File::E_Mode::CREATE);
		
		f.Write(b);
	}
	
	{
		std::array<std::uint8_t, 4> b;
		
		ting::fs::File::Guard fileGuard(f, ting::fs::File::E_Mode::READ);
		
		f.Read(b);
		
		ASSERT_ALWAYS(b[0] == 1)
		ASSERT_ALWAYS(b[1] == 2)
		ASSERT_ALWAYS(b[2] == 3)
		ASSERT_ALWAYS(b[3] == 4)
	}
}
}//~namespace
