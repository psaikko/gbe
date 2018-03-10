cmake_minimum_required(VERSION 2.8.12)
project(libgbe)

add_subdirectory(pybind11)
pybind11_add_module(libgbe wrapper.cpp)
target_link_libraries(libgbe PUBLIC ${CMAKE_SOURCE_DIR}/libgbe.a)
