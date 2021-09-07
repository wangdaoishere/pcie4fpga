cmake_minimum_required(VERSION 2.8)
project( pcie_app_axist_v1.2 )



add_executable( pcie_app_axist_v1.2 main.c xdma_axist_utils.h  dma_utils.h)
include_directories( ${PROJECT_SOURCE_DIR} )
 
