
cmake_minimum_required(VERSION 3.16)

project("game")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB src "source/*.cpp" "source/*.h")


add_executable(game ${src} "source/image.h" "source/utils.cpp" "source/image.cpp" "source/pixel.h" "source/pixel.cpp" "source/display.h" "source/display.cpp" "source/light.cpp" "source/light.h" "source/utils.h" source/color.h source/color.cpp)

set_property(TARGET game PROPERTY CXX_STANDARD 20)

target_include_directories(game PRIVATE source)


add_library(gmtl INTERFACE)
target_include_directories(gmtl INTERFACE extern/gmtl/)


add_subdirectory(extern)
get_property(has_sfml GLOBAL PROPERTY sfml_found)
if (${has_sfml})
    message("sfml found setting has_sfml")
    target_compile_definitions(game PRIVATE HAS_SFML=1)
    target_link_libraries(game sfml-alias-target)
else ()
    message("no sfml found")
endif ()

target_link_libraries(game gmtl)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if (EMSCRIPTEN)

    IF (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo OR CMAKE_BUILD_TYPE MATCHES Release)
        target_compile_options(game PRIVATE -O3 -ffast-math -fno-exceptions)
        target_link_options(game PRIVATE -O3)
    ELSE ()
        target_compile_options(game PRIVATE -gsource-map -g4)
        target_link_options(game PRIVATE -gsource-map -g4)
        message("compiling for debug")
    endif ()


#    set_target_properties(game PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/shell-file.html)


#    target_compile_options(game PRIVATE --shell-file=../shell-file.html)
    target_compile_options(game PRIVATE -fno-rtti)

    target_link_options(game PRIVATE --preload-file /home/henry/renderer/head.obj@/head.obj)
    target_link_options(game PRIVATE -sNO_EXIT_RUNTIME=1)
    target_link_options(game PRIVATE -sEXTRA_EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString'])
#    target_link_options(game PRIVATE --shell-file ${CMAKE_SOURCE_DIR}/shell-file.html)
    target_link_options(game PRIVATE --source-map-base=http://localhost:4000/cmake-build-emscripten/)
    target_link_options(game PRIVATE -sALLOW_MEMORY_GROWTH=1)

    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    set_target_properties (game PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ui)
#    set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/ui)
endif ()
