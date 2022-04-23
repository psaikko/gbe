cmake_minimum_required (VERSION 3.8)

project (gbe)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops -march=native -fPIC -Wall -Wextra -Wno-reorder")

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/wrapper.cpp)

set(LIB_SOURCES ${SOURCES})
list(REMOVE_ITEM LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# library
add_library(libgbe STATIC ${LIB_SOURCES} ${HEADERS})
set_target_properties(libgbe PROPERTIES OUTPUT_NAME "gbe")

# python
find_package(PythonLibs)
target_include_directories(libgbe PUBLIC ${PYTHON_INCLUDE_DIRS})
target_link_libraries(libgbe ${PYTHON_STATIC_LIBRARIES})

# gbe exe
add_executable(gbe ${SOURCES} ${HEADERS})

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

# glfw
find_package(glfw3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
link_libraries(${GLFW_LIBRARY_DIRS})
target_link_libraries(gbe ${GLFW_STATIC_LIBRARIES})

# opengl
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(gbe ${OPENGL_LIBRARIES})

# glew
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(gbe ${GLEW_LIBRARIES})

# openal
find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIRS})
target_link_libraries(gbe openal)
