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

set (CONFIG_YAML config-yaml)
project (${CONFIG_YAML})
cmake_minimum_required (VERSION 2.8)
set (SRC_DIR src)
set (INCL_DIR include)

# Define compile flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Werror")

###
### Define sources
###

set (SOURCES ${SRC_DIR}/config-yaml.cpp ${SRC_DIR}/i2c.c)

###
### Define and locate needed libraries and includes
###
include(FindPkgConfig)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp>=0.3.0)
add_definitions(${YAMLCPP_CFLAGS})
link_directories(${YAMLCPP_LIBRARY_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/${INCL_DIR} ${YAMLCPP_INCLUDE_DIRS})

###
### Create our library
###

add_library (${CONFIG_YAML} SHARED ${SOURCES})

set(YAML_CONFIG_VERSION_MAJOR "0")
set(YAML_CONFIG_VERSION_MINOR "1")
set(YAML_CONFIG_VERSION_PATCH "0")
set(YAML_CONFIG_VERSION "${YAML_CONFIG_VERSION_MAJOR}.${YAML_CONFIG_VERSION_MINOR}.${YAML_CONFIG_VERSION_PATCH}")
set_target_properties(${CONFIG_YAML} PROPERTIES VERSION ${YAML_CONFIG_VERSION})

configure_file(${SRC_DIR}/ops-config-yaml.pc.in ops-config-yaml.pc @ONLY)

target_link_libraries (${CONFIG_YAML} ${YAMLCPP_LIBRARIES})

###
### Installation
###
install(TARGETS ${CONFIG_YAML}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )

install(FILES ${CMAKE_BINARY_DIR}/ops-config-yaml.pc DESTINATION lib/pkgconfig)

install(FILES ${INCL_DIR}/config-yaml.h ${INCL_DIR}/i2c.h
	DESTINATION include)

add_subdirectory(tests)
