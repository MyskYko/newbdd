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

add_library(newbdd_reodebug ${FILENAMES})
target_include_directories(newbdd_reodebug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(newbdd_reodebug PUBLIC aig)
target_compile_options(newbdd_reodebug PRIVATE -DREO_DEBUG)

add_library(newbdd_countones ${FILENAMES})
target_include_directories(newbdd_countones PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(newbdd_countones PUBLIC aig)
target_compile_options(newbdd_countones PUBLIC -DCOUNT_ONES)

file(GLOB TESTFILENAMES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
file(GLOB REOTESTFILENAMES ${CMAKE_CURRENT_SOURCE_DIR}/test/*reotest*.cpp)
file(GLOB TRTESTFILENAMES ${CMAKE_CURRENT_SOURCE_DIR}/test/transduction*.cpp)
list(REMOVE_ITEM TESTFILENAMES ${REOTESTFILENAMES})
list(REMOVE_ITEM TESTFILENAMES ${TRTESTFILENAMES})

foreach(testfilename ${TESTFILENAMES})
  get_filename_component(testname ${testfilename} NAME_WE)
  add_executable(${testname} ${testfilename})
  target_link_libraries(${testname} newbdd)
endforeach()

foreach(testfilename ${REOTESTFILENAMES})
  get_filename_component(testname ${testfilename} NAME_WE)
  add_executable(${testname} ${testfilename})
  target_link_libraries(${testname} newbdd_reodebug)
endforeach()

#foreach(testfilename ${TRTESTFILENAMES})
#  get_filename_component(testname ${testfilename} NAME_WE)
#  add_executable(${testname} ${testfilename})
#  target_link_libraries(${testname} newbdd_countones)
#endforeach()
