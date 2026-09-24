add_subdirectory(EIR)
add_subdirectory(fifo_sched)

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


add_library(sched_demo
   SHARED
      sched_demo.c)

set_target_properties(sched_demo
   PROPERTIES
      OUTPUT_NAME "sched_demo"
      PREFIX ""
      SUFFIX ".so"
)

target_link_libraries(sched_demo
   PRIVATE
      fifo_sched
      eba_common # links to eba_common
      ${CMAKE_DL_LIBS}
)


add_library(circ_buf_demo
   SHARED
      circ_buf_demo.c
)

set_target_properties(circ_buf_demo
   PROPERTIES
      OUTPUT_NAME "circ_buf_demo"
      PREFIX ""
      SUFFIX ".so"
)

target_link_libraries(circ_buf_demo
   PRIVATE
      eba_common # links to eba_common
      eir_common
      ${CMAKE_DL_LIBS})

add_library(stream_demo
   SHARED
      stream_demo.c
)

set_target_properties(stream_demo
   PROPERTIES
      OUTPUT_NAME "stream_demo"
      PREFIX ""
      SUFFIX ".so"
)

target_link_libraries(stream_demo
   PRIVATE
      eba_common # links to eba_common
      eir_common
      ${CMAKE_DL_LIBS})
