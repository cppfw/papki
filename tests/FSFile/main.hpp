#pragma once

#include "../../src/ting/debug.hpp"

#include "tests.hpp"


inline void TestTingFSFile(){
	TestSeekForward::Run();
	TestListDirContents::Run();
	TestHomeDir::Run();
	TestLoadWholeFileToMemory::Run();

	TRACE_ALWAYS(<< "[PASSED]" << std::endl)
}
