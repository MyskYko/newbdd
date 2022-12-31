file(GLOB FILENAMES ${CMAKE_CURRENT_SOURCE_DIR}/aigbdd/*.cpp)

add_library(aigbdd ${FILENAMES})
target_include_directories(aigbdd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/aigbdd)
target_link_libraries(aigbdd PUBLIC newbdd aig)

add_library(aigbdd_reodebug ${FILENAMES})
target_include_directories(aigbdd_reodebug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/aigbdd)
target_link_libraries(aigbdd_reodebug PUBLIC newbdd_reodebug aig)


file(GLOB FILENAMES ${CMAKE_CURRENT_SOURCE_DIR}/transduction/*.cpp)

add_library(transduction ${FILENAMES})
target_include_directories(transduction PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/transduction)
target_link_libraries(transduction PUBLIC newbdd aig)
