cmake_minimum_required(VERSION 3.15)
project(EBA_test_project)


add_executable(eba_boot)

target_sources(eba_boot
   PRIVATE
      eba.c
      eba_utils.c
)

add_subdirectory(eba_programs)
