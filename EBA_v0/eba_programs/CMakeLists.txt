add_subdirectory(EIR)

add_library(hello_world
   SHARED
      hello.c
)

set_target_properties(hello_world
   PROPERTIES
      OUTPUT_NAME "hello_world"
      PREFIX ""
      SUFFIX ".so"
)

target_link_libraries(hello_world
   PRIVATE
      eba_common # links to eba_common
      ${CMAKE_DL_LIBS})
