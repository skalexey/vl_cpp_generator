cmake_minimum_required(VERSION 3.15)

project(cpp_generator C CXX)

macro(setup_module)
	module_message("Setting up module: ${MODULE_NAME}")
	option(TESTS "Add Test target" OFF)
	option(CLIENT "Add vl_jsontocpp executable target" ON)

	# DMBCore dependency
	add_dependency_module(${DEPS}/DataModelBuilder/Core DMBCore)

	# For IDE
	add_headers_to_sources()
	
	# add library
	module_add_library()

	if (TESTS)
		module_message("Adding tests target")
		set(tests_project_name "${MODULE_NAME}_tests")
		add_executable(${tests_project_name} test.cpp)
		target_link_libraries(${tests_project_name} cpp_generator)
	endif ()

	if (CLIENT)
		# add client executable
		add_executable(vlcppgen vl_jsontocpp.cpp)
		target_link_libraries(vlcppgen cpp_generator)
	endif ()
	
	# update dependencies.
	# add_custom_command(
	# 	TARGET vlcppgen PRE_BUILD
	# 	COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/get_dependencies.sh "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_BUILD_TYPE}
	# 	VERBATIM
endmacro()

include(cmake_modules/modules.cmake)
modules_include_all(cmake_modules)
setup_module_project()
