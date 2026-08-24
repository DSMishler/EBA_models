cmake_minimum_required(VERSION 3.15)
project(EBA_test_project)


add_executable(eba_boot)

target_sources(eba_boot
   PRIVATE
      eba.c
      eba_utils.c # TODO: make this a library instead of including it everywhere
)

# this is like including ""-Wl --export-dyamic"" in the makefiles
target_link_libraries(eba_boot
   PRIVATE
      ${CMAKE_DL_LIBS})

add_subdirectory(eba_programs)
