cmake_minimum_required(VERSION 3.21)

# Uncomment the next line if want to experiment with g++ compliation
#set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(cpp_generator C CXX)
option(TESTS "Add Test target" OFF)
option(CLIENT "Add vl_jsontocpp executable target" ON)

if(MSVC)
	# fix __cplusplus macro for MSVC
	add_compile_options(
		"/Zc:__cplusplus"
		"/std:c++latest"
	)
else()
	set(CMAKE_CXX_STANDARD 20)
endif()

if (LOG_ON)
	add_compile_definitions("LOG_ON")
	message("vl_cpp_generator: LOG_ON is defined")
else()
	message("vl_cpp_generator: LOG_ON is not defined")
endif ()
if (LOG_LEVEL_VERBOSE)
	add_compile_definitions("LOG_LEVEL_VERBOSE")
	message("vl_cpp_generator: LOG_LEVEL_VERBOSE is defined")
else()
	message("vl_cpp_generator: LOG_LEVEL_VERBOSE is not defined")
endif ()
if (LOG_LEVEL_DEBUG)
	add_compile_definitions("LOG_LEVEL_DEBUG")
	message("vl_cpp_generator: LOG_LEVEL_DEBUG is defined")
else()
	message("vl_cpp_generator: LOG_LEVEL_DEBUG is not defined")
endif()

# Inherit parent libs logic
set(PARENT_LIBS ${PARENT_LIBS} ${LIBS})
set(PARENT_LIBS ${PARENT_LIBS} PARENT_SCOPE)
if (vl_cpp_generator_INHERIT_PARENT_LIBS)
	set(LIBS ${PARENT_LIBS})
	if (vl_cpp_generator_CMAKE_VERBOSE)
		message("vl_cpp_generator PARENT_LIBS: ${PARENT_LIBS}")
	endif()
endif()
# End of Inherit parent libs logic

# set build folder name variable for any dependency
if (CMAKE_CXX_COMPILER EQUAL "g++")
	set(BUILD_DIR_NAME "Build-g++")
else()
	set(BUILD_DIR_NAME "Build-cmake")
endif()

# set deps variables
set(vl_cpp_generator_DEPS "$ENV{vl_cpp_generator_deps}")
set(vl_cpp_generator_DEPS "${vl_cpp_generator_DEPS}" PARENT_SCOPE)
if ("${DEPS}" STREQUAL "")
	set(DEPS ${vl_cpp_generator_DEPS})
endif()
if ("${DEPS}" STREQUAL "")
	message(SEND_ERROR "vl_cpp_generator: ERROR: DEPS is not specified. Take the default value")
	set(DEPS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)
else()
	message("vl_cpp_generator: DEPS: ${DEPS}")
endif()
set(DEPS ${DEPS} PARENT_SCOPE)

# DMBCore dependency
if ("${DMBCORE_DIR}" STREQUAL "")
	set(DMBCORE_DIR "${DEPS}/DataModelBuilder/Core")
else()
	message("vl_cpp_generator: DMBCORE_DIR: ${DMBCORE_DIR}")
endif()
add_subdirectory("${DMBCORE_DIR}" "${DMBCORE_DIR}/${BUILD_DIR_NAME}")
set(CUSTOM_INCLUDES ${CUSTOM_INCLUDES} ${DMBCORE_INCLUDES})

# Set CPP_GENERATOR_INCLUDES global variable
set(CPP_GENERATOR_INCLUDES
	"${PROJECT_SOURCE_DIR}/include"
	"${CUSTOM_INCLUDES}"
	${INCLUDES}
)
set(CPP_GENERATOR_INCLUDES ${CPP_GENERATOR_INCLUDES} PARENT_SCOPE)

# include all directories
include_directories(
	${CPP_GENERATOR_INCLUDES}
	"${PROJECT_BINARY_DIR}"
	"${CUSTOM_INCLUDES}"
)
message("CPP_GENERATOR_INCLUDES: ${CPP_GENERATOR_INCLUDES}")

# Fetch CPP from external includes
if (NOT "${INCLUDES}" STREQUAL "")
	message("vl_cpp_generator: External INCLUDES variable: ${INCLUDES}")
	foreach(items ${INCLUDES})
		file(GLOB_RECURSE INCLUDES_SRC "${items}/*.cpp")
	endforeach()
	message("vl_cpp_generator: External INCLUDES sources: ${INCLUDES_SRC}")
endif()

# set up sources
file(GLOB SRC "src/*")
file(GLOB HEADERS "include/*")

# add library
add_library(cpp_generator ${SRC} ${HEADERS})

# link libraries
target_link_libraries (cpp_generator DMBCore)

# Link all librariles including inherited
set(cnt 1)
foreach(item ${LIBS})
	if (${cnt} EQUAL 1)
		if (vl_cpp_generator_CMAKE_VERBOSE)	
			message("Link library '${item}'")
		endif()
		set(lib_name ${item})
	elseif(${cnt} EQUAL 2)
		if (vl_cpp_generator_CMAKE_VERBOSE)
			message("Library '${lib_name}' path: ${item}")
		endif()
		set(lib_path ${item})
	elseif(${cnt} EQUAL 3)
		if (vl_cpp_generator_CMAKE_VERBOSE)
			message("Library '${lib_name}' binary path: ${item}")
		endif()
		#add_subdirectory("${lib_path}" "${item}")
		target_link_libraries(vl_cpp_generator ${lib_name})
	endif()
	MATH(EXPR cnt "${cnt}+1")
endforeach()

# configurations
configure_file(
	"${PROJECT_SOURCE_DIR}/cpp_generator_config.h.in"
	"${PROJECT_BINARY_DIR}/cpp_generator_config.h"
)

if (TESTS)
	add_executable(cpp_generatorTest test.cpp)
	target_link_libraries(cpp_generatorTest cpp_generator)
endif ()

if (CLIENT)
	# add client executable
	add_executable(vl_jsontocpp vl_jsontocpp.cpp)
	target_link_libraries(vl_jsontocpp cpp_generator)

	# update dependencies
	add_custom_command(
		TARGET vl_jsontocpp PRE_BUILD
		COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/get_dependencies.sh "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_BUILD_TYPE}
		VERBATIM
)

endif()

