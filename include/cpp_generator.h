#pragma once

#include <string>

namespace vl
{
	class cpp_generator
	{
	public:
		void generate(
			const std::string& input_file_path
			, const std::string& out_dir_path
		);
	};
}
