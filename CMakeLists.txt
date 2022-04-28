cmake_minimum_required (VERSION 3.8)

if (WIN32)
    SET(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

project (gbe)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops -march=native -fPIC -Wall -Wextra -Wno-reorder")
endif()

file(GLOB LIB_SOURCES "src/*.cpp")
list(REMOVE_ITEM LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/wrapper.cpp) # pybind wrapper
list(REMOVE_ITEM LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
list(REMOVE_ITEM LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp)
list(REMOVE_ITEM LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/openal_output.cpp)

# library
add_library(libgbe STATIC ${LIB_SOURCES})
set_target_properties(libgbe PROPERTIES OUTPUT_NAME "gbe")

set(EXE_SOURCES "")
list(APPEND EXE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
list(APPEND EXE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp)
list(APPEND EXE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/openal_output.cpp)

# gbe exe
add_executable(gbe ${EXE_SOURCES})
target_link_libraries(gbe libgbe)

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
if (CMAKE_HOST_WIN32)
    find_package(OpenAL CONFIG REQUIRED)
    target_link_libraries(gbe OpenAL::OpenAL)
else()
    find_package(OpenAL REQUIRED)
    target_link_libraries(gbe openal)
endif()

# getopt
if (CMAKE_HOST_WIN32)
    target_include_directories(gbe PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows/include")
    target_link_libraries(gbe "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows/lib/getopt.lib")
endif()