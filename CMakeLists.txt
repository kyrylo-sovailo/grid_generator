# Header
cmake_minimum_required(VERSION 3.15)
project(grid_generator VERSION 0.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Library
add_library(${CMAKE_PROJECT_NAME} SHARED source/common.cpp source/common_internal.cpp)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:include>")
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

# Example
find_package(GTest)
if(GTest_FOUND)
    add_executable(${CMAKE_PROJECT_NAME}_example example/example.cpp)
    target_link_libraries(${CMAKE_PROJECT_NAME}_example PUBLIC ${CMAKE_PROJECT_NAME})
    target_link_libraries(${CMAKE_PROJECT_NAME}_example PUBLIC GTest::gtest)
    add_custom_target(test WORKING_DIRECTORY COMMAND ${CMAKE_PROJECT_NAME}_example)
endif()

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_SOURCE_DIR}/config.cmake.in"
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME}")

write_basic_package_version_file("${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(TARGETS ${CMAKE_PROJECT_NAME}
    EXPORT ${CMAKE_PROJECT_NAME}_targets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(EXPORT ${CMAKE_PROJECT_NAME}_targets
    FILE "${CMAKE_PROJECT_NAME}Targets.cmake"
    NAMESPACE ${CMAKE_PROJECT_NAME}::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME}")

install(FILES
    "include/${CMAKE_PROJECT_NAME}/point_grid.hxx"
    "include/${CMAKE_PROJECT_NAME}/cell_grid.h"
    "include/${CMAKE_PROJECT_NAME}/cell_grid.hxx"
    "include/${CMAKE_PROJECT_NAME}/common.h"
    "include/${CMAKE_PROJECT_NAME}/common_internal.h"
    "include/${CMAKE_PROJECT_NAME}/point_grid.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME}")

install(FILES "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake" "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME}")

# Export
export(EXPORT ${CMAKE_PROJECT_NAME}_targets
    FILE "${CMAKE_PROJECT_NAME}Targets.cmake"
    NAMESPACE ${CMAKE_PROJECT_NAME}::)

# Uninstallation
add_custom_target(uninstall WORKING_DIRECTORY "${CMAKE_BUILD_DIR}" COMMAND cmake -P "${CMAKE_SOURCE_DIR}/uninstall.cmake" ${CMAKE_PROJECT_NAME})

# Documentation
find_package(Doxygen)
if (Doxygen_FOUND)
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/documentation")
    add_custom_target(doc WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" COMMAND doxygen)
endif()