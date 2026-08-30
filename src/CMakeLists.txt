add_executable(re-vim main.cpp)

target_include_directories(
  re-vim PRIVATE ${CURSES_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
                 ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(re-vim PRIVATE ${CURSES_LIBRARIES})
