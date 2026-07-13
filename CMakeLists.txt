cmake_minimum_required(VERSION 3.24)
project(rmlui_layout_repro CXX)

option(REPRO_SUPERBUILD "Build the repro twice, against upstream RmlUi master and against the layout-measure-cache fix branch" ON)

if(REPRO_SUPERBUILD)
  include(ExternalProject)

  set(REPRO_COMMON_ARGS
    -DREPRO_SUPERBUILD=OFF
    -DCMAKE_BUILD_TYPE=Release
    "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}")

  ExternalProject_Add(repro_upstream
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    CMAKE_ARGS ${REPRO_COMMON_ARGS}
      -DREPRO_EXECUTABLE_NAME=repro_upstream
      -DREPRO_RMLUI_REPOSITORY=https://github.com/mikke89/RmlUi.git
      -DREPRO_RMLUI_TAG=master
    BUILD_ALWAYS ON
    INSTALL_COMMAND ""
    USES_TERMINAL_CONFIGURE ON
    USES_TERMINAL_BUILD ON)

  ExternalProject_Add(repro_fixed
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    CMAKE_ARGS ${REPRO_COMMON_ARGS}
      -DREPRO_EXECUTABLE_NAME=repro_fixed
      -DREPRO_RMLUI_REPOSITORY=https://github.com/geforcefan/RmlUi.git
      -DREPRO_RMLUI_TAG=layout-measure-cache
    BUILD_ALWAYS ON
    INSTALL_COMMAND ""
    USES_TERMINAL_CONFIGURE ON
    USES_TERMINAL_BUILD ON)

  return()
endif()

set(REPRO_EXECUTABLE_NAME repro CACHE STRING "Name of the produced executable")
set(REPRO_RMLUI_REPOSITORY https://github.com/mikke89/RmlUi.git CACHE STRING "RmlUi repository to build against")
set(REPRO_RMLUI_TAG master CACHE STRING "RmlUi branch or tag to build against")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)

FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4)

FetchContent_Declare(rmlui
  GIT_REPOSITORY "${REPRO_RMLUI_REPOSITORY}"
  GIT_TAG "${REPRO_RMLUI_TAG}")

FetchContent_MakeAvailable(glfw rmlui)

find_package(OpenGL REQUIRED)

add_executable(${REPRO_EXECUTABLE_NAME}
  main.cpp
  "${rmlui_SOURCE_DIR}/Backends/RmlUi_Platform_GLFW.cpp"
  "${rmlui_SOURCE_DIR}/Backends/RmlUi_Renderer_GL3.cpp"
  "${rmlui_SOURCE_DIR}/Backends/RmlUi_Backend_GLFW_GL3.cpp")

target_include_directories(${REPRO_EXECUTABLE_NAME} PRIVATE "${rmlui_SOURCE_DIR}/Backends")

target_link_libraries(${REPRO_EXECUTABLE_NAME} PRIVATE RmlUi::RmlUi glfw OpenGL::GL)

target_compile_definitions(${REPRO_EXECUTABLE_NAME} PRIVATE
  REPRO_LABEL="${REPRO_EXECUTABLE_NAME}"
  REPRO_DOCUMENT="${CMAKE_CURRENT_SOURCE_DIR}/repro.rml"
  REPRO_FONT="${rmlui_SOURCE_DIR}/Samples/assets/LatoLatin-Regular.ttf")
