cmake_minimum_required(VERSION 2.6)
project(monitor)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(monitor ${SOURCES})

set_property(TARGET monitor PROPERTY CXX_STANDARD 17)
target_link_libraries(monitor ${CURSES_LIBRARIES} stdc++fs)
target_compile_options(monitor PRIVATE -Wall -Wextra)

option(ENABLE_WERROR "Treat all warnings as errors (-Werror)" OFF)

if (ENABLE_WERROR)
  target_compile_options(monitor PRIVATE -Werror)
endif()
