# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dataflow_shmem_threads)

zephyr_include_directories(Cab) #Add this line
target_sources(app PRIVATE src/main.c)

target_include_directories(app PRIVATE src/Cab) #Add this line
target_sources(app PRIVATE src/Cab/cab.c)  #Add this line



