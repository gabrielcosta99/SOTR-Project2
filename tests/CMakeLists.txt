# tests/CMakeLists.txt
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(stbs_test)

# Add your test and the scheduler source:
target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/stb_scheduler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test_stb_scheduler.c
)

# Include directories:
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
