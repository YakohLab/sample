cmake_minimum_required(VERSION 3.1)
project(sample LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-2.4 REQUIRED)
pkg_check_modules(GTKGLEXT gtkglext-1.0 REQUIRED)

include_directories(${GTKMM_INCLUDE_DIRS} ${GTKGLEXT_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS} ${GTKGLEXT_LIBRARY_DIRS} ${OPENGL_LIBRARY_DIRS}  ${GLUT_LIBRARY_DIRS})

file(GLOB SOURCES src/*.cpp)
add_executable(sample ${SOURCES})
target_link_libraries(sample ${GTKMM_LIBRARIES} ${GTKGLEXT_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GTKMM_LIBRARIES})
