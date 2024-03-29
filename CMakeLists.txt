# Part of the Fluid Corpus Manipulation Project (http://www.flucoma.org/)
# Copyright University of Huddersfield.
# Licensed under the BSD-3 License.
# See license.md file in the project root for full license information.
# This project has received funding from the European Research Council (ERC)
# under the European Union’s Horizon 2020 research and innovation programme
# (grant agreement No 725899).

cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts")

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/release-packaging" CACHE PATH "")

set(FLUID_BRANCH "origin/main" CACHE STRING "Branch to pull flucoma dependencies from")
set(FLUID_PATH "" CACHE PATH "Optional path to the flucoma-core repo; will be downloaded if absent")
option(DOCS "Generate HTML documentaiton" OFF)
set(FLUID_DOCS_PATH "" CACHE PATH "Optional path to flucoma-docs; will be downloaded if absent")

if (APPLE)
  set(CMAKE_XCODE_GENERATE_SCHEME ON)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "")
  #A consequence of targetting 10.8. Needs to be set globally from 10.15 onwards in order for the test program to compile successfully during configure 
  string(APPEND CMAKE_CXX_FLAGS " -stdlib=libc++")
endif()

project (flucoma-cli LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY ON)

include(FetchContent)

FetchContent_Declare(
  flucoma-core
  GIT_REPOSITORY https://github.com/flucoma/flucoma-core.git
  GIT_PROGRESS TRUE
  GIT_TAG ${FLUID_BRANCH}
)

FetchContent_Declare(
  flucoma-docs
  GIT_REPOSITORY https://github.com/flucoma/flucoma-docs.git
  GIT_PROGRESS TRUE
  GIT_TAG ${FLUID_BRANCH}
)

if(FLUID_PATH)
  get_filename_component(
    FETCHCONTENT_SOURCE_DIR_FLUCOMA-CORE ${FLUID_PATH} ABSOLUTE
  )
endif()
FetchContent_MakeAvailable(flucoma-core)
include(flucoma_version)
include(flucoma-buildtools)
include(flucoma-buildtype)

set_if_toplevel(VAR FLUCOMA_CLI_RUNTIME_OUTPUT_DIRECTORY 
                TOPLEVEL "${CMAKE_CURRENT_SOURCE_DIR}/bin"
              SUPERBUILD "${CMAKE_SOURCE_DIR}/cli_binaries/${CMAKE_HOST_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}")

if(DOCS)
  if(FLUID_DOCS_PATH)
    get_filename_component(
      FETCHCONTENT_SOURCE_DIR_FLUCOMA-DOCS ${FLUID_DOCS_PATH} ABSOLUTE
    )
  endif()

  FetchContent_MakeAvailable(flucoma-docs)
  include(FlucomaDocs)
  set(CLI_DOC_OUT "${CMAKE_BINARY_DIR}/cli_ref")
  add_ref_target(cli "Making docs")
  add_custom_target(CLI_MAKE_DOCS ALL DEPENDS MAKE_CLI_REF)  
endif()

set(LOCAL_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(FLUID_CLI_WRAPPER INTERFACE)

target_include_directories (FLUID_CLI_WRAPPER 
  INTERFACE 
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_sources(FLUID_CLI_WRAPPER
  INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include/FluidCLIWrapper.hpp"
)

find_package (Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG TRUE) 
target_link_libraries (FLUID_CLI_WRAPPER INTERFACE Threads::Threads)

get_client_group(NONE client_list)
list(FILTER client_list INCLUDE REGEX Buf.+)
list(FILTER client_list EXCLUDE REGEX "BufThreadDemo")
list(FILTER client_list EXCLUDE REGEX "BufSelect*")
list(FILTER client_list EXCLUDE REGEX "BufCompose")
list(FILTER client_list EXCLUDE REGEX "BufThresh")
list(FILTER client_list EXCLUDE REGEX "BufScale")
list(FILTER client_list EXCLUDE REGEX "BufFlatten")

include(MakeCLIStub)

foreach(client ${client_list}) 
  get_core_client_header(${client} header)   
  get_core_client_class(${client} class)   
  generate_cli_source(
    CLIENTS ${client} 
    HEADERS ${header} 
    CLASSES ${class} 
  )
endforeach()

#install 
set(CLI_INSTALL_PREFIX "." CACHE PATH "Prefix for assembling PD packages")
set(FLUID_PACKAGE_NAME FluidCorpusManipulation CACHE STRING "Name for published package")
set(CLI_PACKAGE_ROOT ${CLI_INSTALL_PREFIX}/${FLUID_PACKAGE_NAME})

install(DIRECTORY ${FLUCOMA_CLI_RUNTIME_OUTPUT_DIRECTORY}/ 
				DESTINATION ${CLI_PACKAGE_ROOT}/bin 
				USE_SOURCE_PERMISSIONS 
				PATTERN "*.ilk" EXCLUDE 
				PATTERN "*.PDB" EXCLUDE)

install(FILES QuickStart.md DESTINATION ${CLI_PACKAGE_ROOT})
install(FILES ${flucoma-core_SOURCE_DIR}/distribution.lic 
				DESTINATION ${CLI_PACKAGE_ROOT} RENAME LICENSE.md)
if(DOCS)
  install(DIRECTORY "${FLUID_CLI_REF_PATH}/" 
				  DESTINATION "${CLI_PACKAGE_ROOT}/docs" 
					FILES_MATCHING REGEX "\\.(html|css)")
endif()
