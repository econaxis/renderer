cmake_minimum_required (VERSION 3.16)

add_subdirectory(zlib-1.2.11)

set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/zlib-1.2.11 ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.2.11)
set(ZLIB_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/zlib-1.2.11/libz.a)

set(PNG_BUILD_ZLIB ON CACHE BOOL "" FORCE)
add_subdirectory(lpng1637)
# target_include_directories(png_static PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extern/zlib-1.2.11/>)
#target_include_directories(png_static PRIVATE "zlib-1.2.11/" ${CMAKE_CURRENT_BINARY_DIR}/zlib-1.2.11/)
#target_include_directories(png PRIVATE "zlib-1.2.11/" ${CMAKE_CURRENT_BINARY_DIR}/zlib-1.2.11/)

target_link_libraries(png_static zlibstatic)

message("PNG STATIC INCLUDE DIR " $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.2.11>)
file(GENERATE OUTPUT debug.txt CONTENT $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.2.11>)
