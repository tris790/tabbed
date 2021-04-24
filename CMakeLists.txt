cmake_minimum_required(VERSION 3.11...3.20)

project(
  Tabbed
  VERSION 0.1
  DESCRIPTION "A global autocomplete software."
  LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  set(CMAKE_CXX_EXTENSIONS OFF)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

endif()

include(FetchContent)
if(${CMAKE_VERSION} VERSION_LESS 3.14)
  include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()

add_subdirectory(src)