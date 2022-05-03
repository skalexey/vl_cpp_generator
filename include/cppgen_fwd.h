#pragma once

#include <string>
// cppw_params
// Conversion parameters
namespace vl
{
	struct cppw_params
	{
		bool use_proto_refs = true;
		bool store_type_id = true;
		bool merge_with_proto = false; // Put everything from 'proto' into its derived object
		std::string cpp_namespace = "cppgen";
		std::string out_dir_path = ".";
	};
}