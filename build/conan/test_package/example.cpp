#include <papki/fs_file.hpp>

#include <iostream>

int main(int argc, const char** argv){
	papki::fs_file f("some.txt");

	std::cout << "Hello papki!" << '\n';
}
