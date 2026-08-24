# add_executable(test_hello)
# 
# target_sources(test_hello
   # PRIVATE
      # test_hello.c
# )

add_library(test_hello
   SHARED
      test_hello.c
      ../eba_utils.c
      ../eba_utils.h
      ../eba.h
)

set_target_properties(test_hello
   PROPERTIES
      OUTPUT_NAME "boot"
      PREFIX ""
      SUFFIX ".so"
)

target_link_libraries(test_hello
   PRIVATE
      ${CMAKE_DL_LIBS})
