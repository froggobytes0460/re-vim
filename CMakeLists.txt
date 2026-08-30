cmake_minimum_required(VERSION 3.23)
project(re-vim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

add_subdirectory(src)
