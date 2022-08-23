#pragma once

#include <string>
// cppw_params
// Conversion parameters
namespace vl
{
	struct cppgenerator_params
	{
		std::string input_file_path;
		std::string out_dir_path = ".";
		std::string json_branch;
		std::string ignore;
		bool print_root = true;
		bool ignore_overloadings = true;
		bool generate_setters = true;
	};

	struct cppw_params
	{
		bool use_proto_refs = true;
		bool store_type_id = true;
		bool merge_with_proto = false; // Put everything from 'proto' into its derived object
		std::string cpp_namespace = "cppgen";
		std::string root_name = "root";
		std::string ignore;
		cppgenerator_params cppgen_params;
	};
}
