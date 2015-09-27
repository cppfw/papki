#pragma once

#include "../../src/ting/debug.hpp"

#include "tests.hpp"


inline void TestTingMemoryFile(){
	TestBasicMemoryFile::Run();

	TRACE_ALWAYS(<< "[PASSED]" << std::endl)
}
