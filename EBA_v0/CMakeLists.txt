cmake_minimum_required(VERSION 3.15)
project(EBA_test_project LANGUAGES C)




add_library(eba_common
   SHARED
      eba_common.c
)
# This makes anything that links this lib in CMake also include eba_common.h
target_include_directories(eba_common
   PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/include
)
set_target_properties(eba_common
   PROPERTIES
      OUTPUT_NAME "eba_common"
      PREFIX ""
      SUFFIX ".so"
   VERSION
      0.0.1
   SOVERISION
      0
)
target_link_libraries(eba_common
   PRIVATE
      ${CMAKE_DL_LIBS} # this is like including ""-Wl --export-dyamic""
)


add_executable(eba)

target_sources(eba
   PRIVATE
      eba.c
)
target_link_libraries(eba
   PRIVATE
      eba_common # links to eba_common
      ${CMAKE_DL_LIBS} # this is like including ""-Wl --export-dyamic""
)


add_subdirectory(eba_boot)
add_subdirectory(eba_programs)

