cmake_minimum_required(VERSION 3.8)
project(newbdd CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-g -Wall -Wextra -Wpedantic -O3) # -DNDEBUG

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

add_library(newbdd_reodebug ${FILENAMES})
target_include_directories(newbdd_reodebug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(newbdd_reodebug PUBLIC aig)
target_compile_options(newbdd_reodebug PRIVATE -DREO_DEBUG)

file(GLOB REOTESTFILENAMES ${CMAKE_CURRENT_SOURCE_DIR}/test/*reotest*.cpp)
foreach(testfilename ${REOTESTFILENAMES})
  get_filename_component(testname ${testfilename} NAME_WE)
  add_executable(${testname}_reodebug ${testfilename})
  target_link_libraries(${testname}_reodebug newbdd_reodebug)
endforeach()

file(GLOB TRTESTFILENAMES ${CMAKE_CURRENT_SOURCE_DIR}/test/transduction*.cpp)
foreach(testfilename ${TRTESTFILENAMES})
  get_filename_component(testname ${testfilename} NAME_WE)
  add_executable(${testname}_cspf ${testfilename})
  target_link_libraries(${testname}_cspf newbdd)
  target_compile_options(${testname}_cspf PRIVATE -DCSPF_ONLY)
endforeach()
