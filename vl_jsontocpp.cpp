#include <utility>
#include <iostream>
#include "cpp_generator.h"
#include <utils/Utils.h>
#include <utils/command_line.h>

int main(int argc, char* argv[])
{
	std::cout << std::boolalpha;
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
		auto arg = argv[i];
		if (i == 1)
			params.input_file_path = arg;
		else if (i == 2)
			params.out_dir_path = arg;
		else
		{
			if (auto opt_val = utils::get_option_value(arg, "-print_root"))
				params.print_root = Utils::ParseBool(opt_val.val);
			else if (auto opt_val = utils::get_option_value(arg, "-branch"))
				params.json_branch = opt_val.val;
			else if (auto opt_val = utils::get_option_value(arg, "-ignore_overloadings"))
				params.ignore_overloadings = Utils::ParseBool(opt_val.val);
			else if (auto opt_val = utils::get_option_value(arg, "-setters"))
				params.generate_setters = Utils::ParseBool(opt_val.val);
		}
	}
	return gen.generate(params);
}

