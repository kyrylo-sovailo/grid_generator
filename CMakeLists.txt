cmake_minimum_required(VERSION 3.0)
project(grid_generator LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

add_library(grid_generator SHARED source/common.cpp source/common_internal.cpp)
target_include_directories(grid_generator PUBLIC "${PROJECT_SOURCE_DIR}/include")

add_executable(grid_generator_example example/example.cpp)
target_link_libraries(grid_generator_example PUBLIC grid_generator)

find_package(Doxygen)
if (Doxygen_FOUND)
    add_custom_target(doc WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" COMMAND sh -c 'if [ ! -d documentation ]\; then mkdir documentation\; fi\; doxygen')
endif()