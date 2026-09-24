
add_library(fifo_sched
   SHARED
      fifo_sched.c
)

target_include_directories(fifo_sched
   PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(fifo_sched
   PROPERTIES
      OUTPUT_NAME "fifo_sched"
      PREFIX ""
      SUFFIX ".so"
   VERSION
      0.0.1
)

target_link_libraries(fifo_sched
   PRIVATE
      eba_common # links to eba_common
      ${CMAKE_DL_LIBS}
)
