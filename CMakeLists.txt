cmake_minimum_required(VERSION 3.11)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/release-packaging" CACHE PATH "")
# message(FATAL_ERROR ${CMAKE_INSTALL_PREFIX})

set(FLUID_PATH "" CACHE PATH "Optional path to the Fluid Decomposition repo; will be downloaded if absent")
# set(FLUID_M_PATH "" CACHE PATH "Optional path to the Fluid fluid_manipulation repo")
option(DOCS "Generate HTML documentaiton" OFF)
set(FLUID_PARAMDUMP_PATH "" CACHE PATH "Optional path to fluid_paramdump; will be downloaded if absent")

if (APPLE)
	set(CMAKE_OSX_ARCHITECTURES x86_64)
  set(CMAKE_XCODE_GENERATE_SCHEME ON)
endif()

project (flucoma-cli LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY ON)

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

include(FetchContent)

FetchContent_Declare(
  fluid_decomposition
  GIT_REPOSITORY https://bitbucket.org/flucoma/fluid_decomposition.git
  GIT_PROGRESS TRUE
)

FetchContent_Declare(
  fluid_paramdump
  GIT_REPOSITORY https://bitbucket.org/flucoma/fluid_paramdump.git
  GIT_PROGRESS TRUE
)

if(FLUID_PATH)
  get_filename_component(
    FETCHCONTENT_SOURCE_DIR_FLUID_DECOMPOSITION ${FLUID_PATH} ABSOLUTE
  )
endif()

FetchContent_GetProperties(fluid_decomposition)
if(NOT fluid_decomposition_POPULATED)
  FetchContent_Populate(fluid_decomposition)
  add_subdirectory(${fluid_decomposition_SOURCE_DIR} ${fluid_decomposition_BINARY_DIR})
endif()

if(DOCS)

  if(FLUID_PARAMDUMP_PATH)
    get_filename_component(
      FETCHCONTENT_SOURCE_DIR_FLUID_PARAMDUMP ${FLUID_PARAMDUMP_PATH} ABSOLUTE
    )
  endif()

  FetchContent_GetProperties(fluid_paramdump)
  if(NOT fluid_paramdump_POPULATED)
    FetchContent_Populate(fluid_paramdump)
    file(GLOB_RECURSE DOC_SOURCE RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${fluid_paramdump_SOURCE_DIR}/**/*.cpp" )
    source_group("\\XML Doc Gen" FILES ${DOC_SOURCE})
    add_subdirectory(${fluid_paramdump_SOURCE_DIR} ${fluid_paramdump_BINARY_DIR})
  endif()

  add_custom_target(CLI_MAKE_DOCS DEPENDS GENERATE_HTML)  
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

SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)
foreach (project_dir ${PROJECT_DIRS})
	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/${project_dir}/CMakeLists.txt")
		message("Generating: ${project_dir}")
		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/${project_dir})
	endif ()
endforeach ()

#install 

set(CLI_INSTALL_PREFIX "." CACHE PATH "Prefix for assembling PD packages")
set(FLUID_PACKAGE_NAME FluidCorpusManipulation CACHE STRING "Name for published package")
set(CLI_PACKAGE_ROOT ${CLI_INSTALL_PREFIX}/${FLUID_PACKAGE_NAME})

# message(FATAL_ERROR ${CMAKE_INSTALL_PREFIX})

install(DIRECTORY "${CMAKE_BINARY_DIR}/bin" DESTINATION ${CLI_PACKAGE_ROOT} PATTERN "*.ilk" EXCLUDE PATTERN "*.PDB" EXCLUDE)

install(FILES license.md QuickStart.md DESTINATION ${CLI_PACKAGE_ROOT})

if(DOCS)
  install(DIRECTORY "${CMAKE_BINARY_DIR}/html/" DESTINATION "${CLI_PACKAGE_ROOT}/docs" FILES_MATCHING REGEX "\\.(html|css)")
endif()
