#include <iostream>
#include <DMBCore.h>
#include "cpp_writer.h"
#include "cpp_generator.h"

#ifdef LOG_ON
	#include "Log.h"
	LOG_TITLE("spell_options")
	LOG_STREAM([]() -> std::ostream& { return std::cout; })
	SET_LOG_DEBUG(true)
#endif

namespace vl
{
	void cpp_generator::generate(
		const std::string& input_file_path
		, const std::string& out_dir_path
	)
	{
		dmb::Model m;
		if (m.Load(input_file_path))
		{
			cppw_params params;
			params.out_dir_path = out_dir_path;
			cpp_writer wr(m.GetTypeResolver(), params);
			m.GetData().Accept(wr);
		}
		else
		{
			std::cout << "Can't load JSON '" << input_file_path << "'\n";
		}
	}
}
