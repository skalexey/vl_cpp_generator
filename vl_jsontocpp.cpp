#include <utility>
#include <iostream>
#include "cpp_generator.h"
#include <utils/string_utils.h>
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
				params.print_root = utils::parse_bool(opt_val.val);
			else if (auto opt_val = utils::get_option_value(arg, "-branch"))
				params.json_branch = opt_val.val;
			else if (auto opt_val = utils::get_option_value(arg, "-itnore"))
				params.ignore = opt_val.val;
			else if (auto opt_val = utils::get_option_value(arg, "-ignore_overloadings"))
				params.ignore_overloadings = utils::parse_bool(opt_val.val);
			else if (auto opt_val = utils::get_option_value(arg, "-setters"))
				params.generate_setters = utils::parse_bool(opt_val.val);
		}
	}
	return gen.generate(params);
}

