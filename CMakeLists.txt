cmake_minimum_required(VERSION 3.8)
project(newbdd CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-g -O3 -Wall -Wextra) # -DNDEBUG

add_subdirectory(lib)

file(GLOB FILENAMES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(newbdd ${FILENAMES})
target_include_directories(newbdd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(newbdd PUBLIC aig)

file(GLOB TESTFILENAMES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
foreach(testfilename ${TESTFILENAMES})
  get_filename_component(testname ${testfilename} NAME_WE)
  add_executable(${testname} ${testfilename})
  target_link_libraries(${testname} newbdd)
endforeach()
