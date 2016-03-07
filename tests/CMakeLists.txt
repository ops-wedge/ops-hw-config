# (c) Copyright 2015 Hewlett Packard Enterprise Development LP
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.

cmake_minimum_required (VERSION 2.8)

project(CFG_YAML_LIB_UT CXX C)

# CFG_YAML Library unit tests.
set(CFG_YAML_UT_EXE cfg_yaml_ut)

configure_file (${PROJECT_SOURCE_DIR}/cfg_yaml_ut.h.in
		${PROJECT_BINARY_DIR}/cfg_yaml_ut.h)

# Source files to build unit tests
set (SOURCES cfg_yaml_ut.cpp i2c_fakes.c)

# Rules to locate needed libraries
include(FindPkgConfig)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp>=0.3.0)
pkg_check_modules(GTEST REQUIRED gtest>=1.7.0)

add_definitions(${YAMLCPP_CFLAGS})
link_directories(${YAMLCPP_LIBRARY_DIRS})
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${YAMLCPP_INCLUDE_DIRS})

# Rules to locate needed libraries
add_executable(${CFG_YAML_UT_EXE} ${SOURCES})

target_link_libraries(${CFG_YAML_UT_EXE} -pthread
                      ${GTEST_LIBRARIES} ${YAMLCPP_LIBRARIES})
