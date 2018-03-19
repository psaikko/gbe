cmake_minimum_required (VERSION 2.6)

project (gbe)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17 -O2 -funroll-loops -march=native -fPIC -Wall -Wextra -Wno-reorder")
file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h")
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/wrapper.cpp)

set(LIB_SOURCES ${SOURCES})
list(REMOVE_ITEM LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(gbe ${SOURCES} ${HEADERS})
add_library(libgbe STATIC ${LIB_SOURCES} ${HEADERS})
set_target_properties(libgbe PROPERTIES OUTPUT_NAME "gbe")

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

# glfw
find_package(glfw3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
link_libraries(${GLFW_LIBRARY_DIRS})

# opengl
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# glew
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

# openal
find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIRS})

target_link_libraries(gbe ${GLFW_STATIC_LIBRARIES})
target_link_libraries(gbe ${GLEW_LIBRARIES})
target_link_libraries(gbe ${OPENGL_LIBRARIES})
target_link_libraries(gbe openal)
