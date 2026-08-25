add_library(eir_common
   SHARED
      eir_common.c
)

target_include_directories(eir_common
   PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(eir_common
   PROPERTIES
      OUTPUT_NAME "eir_common"
      PREFIX ""
      SUFFIX ".so"
   VERSION
      0.0.1
)

target_link_libraries(eir_common
   PRIVATE
      eba_common # links to eba_common
      ${CMAKE_DL_LIBS}
)



add_library(EIR
   SHARED
      eirtest.c
      interpreter.c
      reader.c
)

set_target_properties(EIR
   PROPERTIES
      OUTPUT_NAME "EIR"
      PREFIX ""
      SUFFIX ".so"
   VERSION
      0.0.1
)

target_link_libraries(EIR
   PRIVATE
      eir_common # links to eba_common
      eba_common # links to eba_common
      ${CMAKE_DL_LIBS})
