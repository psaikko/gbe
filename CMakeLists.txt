cmake_minimum_required (VERSION 3.8)

project (gbe)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops -march=native -fPIC -Wall -Wextra -Wno-reorder")
endif()

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/wrapper.cpp)

set(LIB_SOURCES ${SOURCES})
list(REMOVE_ITEM LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
list(REMOVE_ITEM LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp)

# library
add_library(libgbe STATIC ${LIB_SOURCES} ${HEADERS})
set_target_properties(libgbe PROPERTIES OUTPUT_NAME "gbe")

# gbe exe
add_executable(gbe ${SOURCES} ${HEADERS})

# glfw
find_package(glfw3 REQUIRED)
target_link_libraries(gbe glfw)

# opengl
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
target_link_libraries(gbe OpenGL::GL)

# glew
find_package(GLEW REQUIRED)
target_link_libraries(gbe GLEW::GLEW)

# openal
find_package(OpenAL REQUIRED)
target_link_libraries(gbe openal)
target_link_libraries(libgbe openal)
