#include <iostream>
#include "cpp_generator.h"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "To few arguments\n";
		return 1;
	}
	// 1 - input file
	// 2 - output directory path
	std::cout << "vl_jsontocpp project\n";
	vl::cpp_generator gen;
	std::string out_dir = argc >= 2 ? argv[2] : ".";
	gen.generate(argv[1], out_dir);
	return 0;
}

