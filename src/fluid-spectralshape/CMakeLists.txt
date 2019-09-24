cmake_minimum_required(VERSION 3.8)
get_filename_component(PROG ${CMAKE_CURRENT_LIST_DIR} NAME_WE)
message("Configuring ${PROG}")
set(FILENAME ${PROG}.cpp)

add_executable(
  ${PROG}
  ${FILENAME}
)

include(${CMAKE_CURRENT_LIST_DIR}/../../scripts/target_post.cmake)
