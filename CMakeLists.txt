
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
macro(app_set_runner_args)
  board_runner_args(dfu-util "--dfuse-modifiers=force:mass-erase")
  board_runner_args(pyocd "--erase")
  board_runner_args(nrfjprog "--erase")
endmacro()
#add_subdirectory(EXTERNAL/Zephyr EXCLUDE_FROM_ALL)
 
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_project) #Project directory


FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE 
${ZEPHYR_BASE}/subsys/fs/nvs,
${ZEPHYR_BASE}/include/zephyr/portability)
