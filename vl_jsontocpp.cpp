#include <utility>
#include <iostream>
#include "cpp_generator.h"
#include <utils/Utils.h>
#include <utils/command_line.h>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Too few arguments\n";
		return 1;
	}
	// 1 - input file
	// 2 - output directory path
	std::cout << "vl_jsontocpp project\n";
	vl::cpp_generator gen;
	vl::cppgenerator_params params;
	
	for (int i = 0; i < argc; i++)
	{
		if (i == 1)
			params.input_file_path = argv[i];
		else if (i == 2)
			params.out_dir_path = argv[i];
		else
		{
			if (auto opt_val = utils::get_option_value(argv[i], "-print_root"))
				params.print_root = Utils::ParseBool(opt_val.val);
			if (auto opt_val = utils::get_option_value(argv[i], "-branch"))
				params.json_branch = opt_val.val;
			if (auto opt_val = utils::get_option_value(argv[i], "-ignore_overloadings"))
				params.ignore_overloadings = Utils::ParseBool(opt_val.val);
			if (auto opt_val = utils::get_option_value(argv[i], "-setters"))
				params.generate_setters = Utils::ParseBool(opt_val.val);
		}
	}
	return gen.generate(params);
}

