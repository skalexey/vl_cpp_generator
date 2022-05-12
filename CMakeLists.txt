cmake_minimum_required(VERSION 3.21)

# Uncomment the next line if want to experiment with g++ compliation
#set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

project(cpp_generator)
option(TESTS "Add Test target" OFF)
option(CLIENT "Add vl_jsontocpp executable target" ON)

if ("${DEPS}" STREQUAL "")
	message("WARNING: DEPS is not specified. Take the default value")
	set(DEPS "${PROJECT_SOURCE_DIR}/..")
endif()
message("vl_cpp_generator: DEPS: ${DEPS}")

set(DMBCORE_DIR "${DEPS}/DataModelBuilder/Core")
message("vl_cpp_generator DMBCORE_DIR: '${DMBCORE_DIR}'")
#message("cpp_generator CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

if (CMAKE_CXX_COMPILER EQUAL "g++")
	set(BUILD_DIR_NAME "Build-g++")
else ()
	set(BUILD_DIR_NAME "Build-cmake")
endif ()

add_subdirectory("${DMBCORE_DIR}" "${DMBCORE_DIR}/${BUILD_DIR_NAME}")

set(CPP_GENERATOR_INCLUDES
	${DMBCORE_INCLUDES}
	"${PROJECT_SOURCE_DIR}/include"
	${INCLUDES}
)
set(CPP_GENERATOR_INCLUDES ${CPP_GENERATOR_INCLUDES} PARENT_SCOPE)

include_directories(
	${CPP_GENERATOR_INCLUDES}
	"${PROJECT_BINARY_DIR}"
)
message("CPP_GENERATOR_INCLUDES: ${CPP_GENERATOR_INCLUDES}")

file(GLOB SRC "src/*")
file(GLOB HEADERS "include/*")
add_library(cpp_generator ${SRC} ${HEADERS})

target_link_libraries (cpp_generator DMBCore)

configure_file(
	"${PROJECT_SOURCE_DIR}/cpp_generator_config.h.in"
	"${PROJECT_BINARY_DIR}/cpp_generator_config.h"
)

if (TESTS)
	add_executable(cpp_generatorTest test.cpp)
	target_link_libraries(cpp_generatorTest cpp_generator)
endif ()

if (CLIENT)
	add_executable(vl_jsontocpp vl_jsontocpp.cpp)
	target_link_libraries(vl_jsontocpp cpp_generator)
endif()

add_custom_command(
    TARGET vl_jsontocpp PRE_BUILD
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/get_dependencies.sh ${CMAKE_BUILD_TYPE} "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
)