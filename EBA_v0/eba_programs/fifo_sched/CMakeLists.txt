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


add_library(write_length
   SHARED
      write_length.c
)

set_target_properties(write_length
   PROPERTIES
      OUTPUT_NAME "write_length"
      PREFIX ""
      SUFFIX ".so"
)

target_link_libraries(write_length
   PRIVATE
      eba_common
      fifo_sched
      ${CMAKE_DL_LIBS}
)

add_library(sched_noise
   SHARED
      sched_noise.c
)

set_target_properties(sched_noise
   PROPERTIES
      OUTPUT_NAME "sched_noise"
      PREFIX ""
      SUFFIX ".so"
)

target_link_libraries(sched_noise
   PRIVATE
      eba_common
      fifo_sched
      ${CMAKE_DL_LIBS}
)
