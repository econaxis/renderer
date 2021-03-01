cmake_minimum_required (VERSION 3.16)

add_subdirectory(zlib-1.2.11)

set(ZLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/zlib-1.2.11 ${CMAKE_BINARY_DIR}/extern/zlib-1.2.11)
set(ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/extern/zlib-1.2.11/zlibstaticd.lib)
add_subdirectory(lpng1637)


