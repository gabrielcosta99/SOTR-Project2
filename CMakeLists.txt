cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nrf_connect_sdk_fundamentals)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/stb_scheduler.c)
target_sources(app PRIVATE src/RTDB.c)
target_sources(app PRIVATE src/functions.c)
# target_sources(app PRIVATE tests/stbs_test.c)