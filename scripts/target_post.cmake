# Part of the Fluid Corpus Manipulation Project (http://www.flucoma.org/)
# Copyright University of Huddersfield.
# Licensed under the BSD-3 License.
# See license.md file in the project root for full license information.
# This project has received funding from the European Research Council (ERC)
# under the European Union’s Horizon 2020 research and innovation programme
# (grant agreement No 725899).


if(MSVC)
  foreach(flag_var
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
endif()

target_link_libraries(${PROG}
  PRIVATE
  FLUID_DECOMPOSITION
  FLUID_CLI_WRAPPER
)

target_include_directories(
  ${PROG}
  PRIVATE 
  "${FLUID_VERSION_PATH}"
)

if (APPLE)
  #targeting <= 10.9, need to explicitly set libc++
  target_compile_options(${PROG} PRIVATE -stdlib=libc++)
  target_link_libraries(${PROG} PRIVATE -stdlib=libc++)
endif()

#set AVX, or whatever
if(DEFINED FLUID_ARCH)  
    target_compile_options(${PROG} PRIVATE ${FLUID_ARCH})
endif()

if(MSVC)
  target_compile_options(${PROG} PRIVATE  -D_USE_MATH_DEFINES /W3)
else()
  target_compile_options(${PROG} PRIVATE -Wall -Wextra -Wpedantic -Wreturn-type -Wno-conversion)
endif(MSVC)

set_target_properties(
  ${PROG}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${FLUCOMA_CLI_RUNTIME_OUTPUT_DIRECTORY}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${FLUCOMA_CLI_RUNTIME_OUTPUT_DIRECTORY}
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${FLUCOMA_CLI_RUNTIME_OUTPUT_DIRECTORY}
)

get_property(HEADERS TARGET FLUID_DECOMPOSITION PROPERTY INTERFACE_SOURCES)
source_group(TREE "${flucoma-core_SOURCE_DIR}/include" FILES ${HEADERS})
