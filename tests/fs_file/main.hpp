#pragma once

#include <utki/debug.hpp>

#include "tests.hpp"

inline void TestTingFSFile(){
	TestSeekForward::Run();
	TestListDirContents::Run();
	TestHomeDir::Run();
	TestLoadWholeFileToMemory::Run();
}
