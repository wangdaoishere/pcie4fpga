cmake_minimum_required(VERSION 2.8)
project( pcie_app_v1.1 )



add_executable( pcie_app_v1.1 main.c xdma_utils.h  dma_utils.h)
include_directories( ${PROJECT_SOURCE_DIR} )
 
