
cmake_minimum_required (VERSION 3.16)

project ("game")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB src "source/*.cpp" "source/*.h")




add_executable (game ${src} "source/image.h" "source/image.cpp" "source/pixel.h" "source/pixel.cpp" "source/display.h" "source/display.cpp" "source/VertexCalculator.cpp" "source/VertexCalculator.h")

set_property(TARGET game PROPERTY CXX_STANDARD 20)

target_include_directories(game PRIVATE source)

add_subdirectory(extern)

if(MSVC)
    message("MSVC")

    #Delete /W3
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    target_compile_options(game PRIVATE /W4)
else()
    target_compile_options(game PRIVATE -Og -pipe -Wno-language-extension-token -fno-elide-type -fdiagnostics-show-template-tree -Wall -Wextra -Wpedantic -Wvla -Wextra-semi -Wnull-dereference -MP)
endif()


# Disable precompiled headers, they are annoying
set (PRECOMP_HEADERS FALSE)
if(PRECOMP_HEADERS)
    #  precompiled headers doesn't work with intellisense when using GNU CLI clang++
    message("enabling precompiled headers")
    target_precompile_headers(game PRIVATE [["pch.h"]])
endif()

if(WIN32)
    # Set INCLUDE and LIBPATH
    message("WIN32 CONFIG")
    # target_include_directories(game SYSTEM PRIVATE "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.27.29110/include")
    # set(ENV{LIBPATH} "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.27.29110/ATLMFC/lib/x64;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.27.29110/lib/x64;C:/Program Files (x86)/Windows Kits/10/lib/10.0.18362.0/ucrt/x64;C:/Program Files (x86)/Windows Kits/10/lib/10.0.18362.0/um/x64;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.27.29110/ATLMFC/lib/x64;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.27.29110/lib/x64;C:/Program Files (x86)/Windows Kits/10/lib/10.0.18362.0/ucrt/x64;C:/Program Files (x86)/Windows Kits/10/lib/10.0.18362.0/um/x64")
endif()

add_library(gmtl INTERFACE)
target_include_directories(gmtl INTERFACE extern/gmtl/)


add_library(png++ INTERFACE)
target_include_directories(png++ INTERFACE extern/png++/ extern/lpng1637/)

# add_subdirectory(extern/lpng1637)

if(TARGET png++)   
    message("png++ is a target")
endif()
if(TARGET zlibtest)   
    message("zlibtest is a target")
endif()
if(TARGET zlib)   
    message("zlib is a target")
endif()

# set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR ${CMAKE_SOURCE_DIR}/extern/sfml/build/install/lib/cmake/SFML)
find_package(SFML 2.5.1 COMPONENTS graphics system REQUIRED)
target_link_libraries(game gmtl png++ png_static zlibstatic sfml-graphics sfml-window)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
