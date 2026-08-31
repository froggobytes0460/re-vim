set(SOURCES main.cpp terminal.cpp)

add_executable(re-vim ${SOURCES})

target_include_directories(
  re-vim PRIVATE ${CURSES_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}
                 ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(re-vim PRIVATE ${CURSES_LIBRARIES})
