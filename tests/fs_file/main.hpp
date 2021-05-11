#pragma once

#include <utki/debug.hpp>

#include "tests.hpp"


inline void TestTingFSFile(){
	TestSeekForward::Run();
	// TODO: re-enable the test when bug in qemu is fixed (https://bugs.launchpad.net/qemu/+bug/1805913)
	utki::log([](auto&o){o << "WARNING: TestListDirContents test is not run due to bug in qemu" << std::endl;});
	// TestListDirContents::Run();
	TestHomeDir::Run();
	TestLoadWholeFileToMemory::Run();
}
