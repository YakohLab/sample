cmake_minimum_required(VERSION 3.1)
project(sample LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-2.4)
pkg_check_modules(GTKGLEXT gtkglext-1.0)
include_directories(${GTKMM_INCLUDE_DIRS} ${GTKGLEXT_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS} ${GTKGLEXT_LIBRARY_DIRS})

file(GLOB SOURCES *.cpp)
add_executable(sample ${SOURCES})
target_link_libraries(sample ${GTKMM_LIBRARIES} ${GTKGLEXT_LIBRARIES} glut)
