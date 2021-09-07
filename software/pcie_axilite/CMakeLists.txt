cmake_minimum_required(VERSION 2.8)
project( pcie_axilite)



add_compile_options(-std=c++11  -lpthread)

add_executable( pcie_axilite main.cpp pcie_fun.h )
include_directories( ${PROJECT_SOURCE_DIR} )
target_link_libraries( pcie_axilite  -pthread ) 
