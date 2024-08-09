cmake_minimum_required(VERSION 3.16)
project(OLLAMA_HPP VERSION 0.9.0)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)
set(CMAKE_CXX_STANDARD 11)

add_compile_options(-Wall -Wpedantic)
add_compile_options(-m64)
#add_compile_options(-O3)

#IF(NOT CMAKE_BUILD_TYPE)
#  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
#ENDIF(NOT CMAKE_BUILD_TYPE)

if (WIN32)
  if (MINGW)
    add_definitions(-D_WIN32_WINNT=0x0A00)
  endif(MINGW)
    link_libraries(wsock32 ws2_32)
endif (WIN32)

add_executable(Example examples/main.cpp)
add_executable(Test test/test.cpp)

target_include_directories(Example  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(Test  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/singleheader/)
