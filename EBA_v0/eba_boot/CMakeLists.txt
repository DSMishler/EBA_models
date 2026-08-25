# add_executable(test_hello)
# 
# target_sources(test_hello
   # PRIVATE
      # test_hello.c
# )

add_library(test_hello
   SHARED
      test_hello.c
)

set_target_properties(test_hello
   PROPERTIES
      OUTPUT_NAME "test_boot"
      PREFIX ""
      SUFFIX ".so"
)

target_link_libraries(test_hello
   PRIVATE
      eba_common # links to eba_common
      ${CMAKE_DL_LIBS})


add_library(shell_boot
   SHARED
      shell_boot.c
)

set_target_properties(shell_boot
   PROPERTIES
      OUTPUT_NAME "boot"
      PREFIX ""
      SUFFIX ".so"
)

target_link_libraries(shell_boot
   PRIVATE
      eba_common # links to eba_common
      ${CMAKE_DL_LIBS})
