
cmake_minimum_required (VERSION 3.16)

project ("game")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB src "source/*.cpp" "source/*.h")




add_executable (game ${src} "source/image.h" "source/image.cpp" "source/pixel.h" "source/pixel.cpp" "source/display.h" "source/display.cpp" "source/light.cpp" "source/light.h" "source/utils.h" source/color.h source/color.cpp)

set_property(TARGET game PROPERTY CXX_STANDARD 20)

target_include_directories(game PRIVATE source)


add_library(gmtl INTERFACE)
target_include_directories(gmtl INTERFACE extern/gmtl/)


add_subdirectory(extern)
target_link_libraries(game gmtl sfml-alias-target)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

