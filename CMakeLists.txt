cmake_minimum_required(VERSION 3.23)
project(re-vim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE AND NOT CMAKE_OSX_SYSROOT)
  execute_process(
    COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
endif()

set(CURSES_NEED_NCURSES TRUE)
if(APPLE AND NOT CMAKE_PREFIX_PATH)
  find_program(BREW_EXECUTABLE brew)
  if(BREW_EXECUTABLE)
    execute_process(
      COMMAND ${BREW_EXECUTABLE} --prefix ncurses
      OUTPUT_VARIABLE NCURSES_BREW_PREFIX
      OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(NCURSES_BREW_PREFIX)
      list(PREPEND CMAKE_PREFIX_PATH ${NCURSES_BREW_PREFIX})
    endif()
  endif()
endif()

find_package(Curses QUIET)

if(NOT CURSES_FOUND)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(EXISTS "/etc/debian_version")
      set(_hint "sudo apt install libncurses-dev")
    elseif(EXISTS "/etc/fedora-release")
      set(_hint "sudo dnf install ncurses-devel")
    elseif(EXISTS "/etc/arch-release")
      set(_hint "sudo pacman -S ncurses")
    elseif(EXISTS "/etc/alpine-release")
      set(_hint "apk add ncurses-dev")
    else()
      set(_hint "install your distro's ncurses development package")
    endif()
  elseif(APPLE)
    set(_hint "brew install ncurses")
  elseif(WIN32)
    set(_hint
        "MSYS2: pacman -S mingw-w64-x86_64-ncurses or vcpkg install pdcurses")
  else()
    set(_hint "install ncurses development headers")
  endif()

  message(FATAL_ERROR "ncurses development files not found.\n"
                      "Install them with:\n    ${_hint}\n" "Then re-run cmake.")
endif()

include(FetchContent)

FetchContent_Declare(argparse
                     GIT_REPOSITORY https://github.com/p-ranav/argparse.git)
FetchContent_MakeAvailable(argparse)

add_subdirectory(src)
