# grab glfw
include(FetchContent)

# Disable optional glfw targets
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
   glfw
   GIT_REPOSITORY https://github.com/glfw/glfw.git
   GIT_TAG 3.5.1
   GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)



# copy over all the example code
file(COPY
   "${CMAKE_CURRENT_SOURCE_DIR}/EIR_code"
   DESTINATION
      "${CMAKE_CURRENT_BINARY_DIR}"
)

# file(GLOB
   # EIR_CODE
   # CONFIGURE_DEPENDS
      # "${CMAKE_CURRENT_SOURCE_DIR}/EIR_code/*/*.EIR"
# )


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



add_library(eir_cleanup
   SHARED
      eir_cleanup.c
)

set_target_properties(eir_cleanup
   PROPERTIES
      OUTPUT_NAME "eir_cleanup"
      PREFIX ""
      SUFFIX ".so"
)

target_link_libraries(eir_cleanup
   PRIVATE
      eir_common
      eba_common
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
      eir_common
      eba_common
      ${CMAKE_DL_LIBS}
)

################################################################################

add_library(EIR_bufreq
   SHARED
      modules/bufreq.c
)

target_include_directories(EIR_bufreq
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(EIR_bufreq
   PROPERTIES
      OUTPUT_NAME "bufreq"
      PREFIX ""
      SUFFIX ".so"
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs
)

target_link_libraries(EIR_bufreq
   PRIVATE
      eba_common # links to eba_common
      eir_common
      ${CMAKE_DL_LIBS})

add_library(EIR_memop
   SHARED
      modules/memop.c
)

target_include_directories(EIR_memop
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(EIR_memop
   PROPERTIES
      OUTPUT_NAME "memop"
      PREFIX ""
      SUFFIX ".so"
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs
)

target_link_libraries(EIR_memop
   PRIVATE
      eba_common # links to eba_common
      eir_common
      ${CMAKE_DL_LIBS})

add_library(EIR_invoke
   SHARED
      modules/invoke.c
)

target_include_directories(EIR_invoke
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(EIR_invoke
   PROPERTIES
      OUTPUT_NAME "invoke"
      PREFIX ""
      SUFFIX ".so"
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs
)

target_link_libraries(EIR_invoke
   PRIVATE
      eba_common # links to eba_common
      eir_common
      ${CMAKE_DL_LIBS})

add_library(EIR_mathop
   SHARED
      modules/mathop.c
)

target_include_directories(EIR_mathop
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(EIR_mathop
   PROPERTIES
      OUTPUT_NAME "mathop"
      PREFIX ""
      SUFFIX ".so"
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs
)

target_link_libraries(EIR_mathop
   PRIVATE
      eba_common # links to eba_common
      eir_common
      ${CMAKE_DL_LIBS})

add_library(EIR_cmp
   SHARED
      modules/cmp.c
)

target_include_directories(EIR_cmp
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(EIR_cmp
   PROPERTIES
      OUTPUT_NAME "cmp"
      PREFIX ""
      SUFFIX ".so"
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs
)

target_link_libraries(EIR_cmp
   PRIVATE
      eba_common # links to eba_common
      eir_common
      ${CMAKE_DL_LIBS})

add_library(EIR_print
   SHARED
      modules/print.c
)

target_include_directories(EIR_print
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(EIR_print
   PROPERTIES
      OUTPUT_NAME "print"
      PREFIX ""
      SUFFIX ".so"
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs
)

target_link_libraries(EIR_print
   PRIVATE
      eba_common # links to eba_common
      eir_common
      ${CMAKE_DL_LIBS})

add_library(EIR_log
   SHARED
      modules/log.c
)

target_include_directories(EIR_log
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(EIR_log
   PROPERTIES
      OUTPUT_NAME "log"
      PREFIX ""
      SUFFIX ".so"
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs
)

target_link_libraries(EIR_log
   PRIVATE
      eba_common # links to eba_common
      eir_common
      ${CMAKE_DL_LIBS})

add_library(EIR_scaffold
   SHARED
      modules/scaffold.c
      ${CMAKE_CURRENT_SOURCE_DIR}/tools/graphics/glad/src/gl.c
)

target_include_directories(EIR_scaffold
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/tools/graphics/glad/include
)

find_package(OpenGL REQUIRED)
# find_package(glfw3 REQUIRED)

set_target_properties(EIR_scaffold
   PROPERTIES
      OUTPUT_NAME "scaffold"
      PREFIX ""
      SUFFIX ".so"
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs
)

target_link_libraries(EIR_scaffold
   PRIVATE
      eba_common # links to eba_common
      eir_common
      glfw
      OpenGL::GL
      ${CMAKE_DL_LIBS})

# target_link_options(EIR_scaffold
   # PRIVATE
      # "-lglfw -lGL"
# )
