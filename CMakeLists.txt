
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


#add_subdirectory(extern)
get_property(has_sfml GLOBAL PROPERTY sfml_found)
if(${has_sfml})
    message("sfml found setting has_sfml")
    target_compile_definitions(game PRIVATE HAS_SFML=1)
    target_link_directories(game sfml-alias-target)
endif()

target_link_libraries(game gmtl)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
target_compile_options(game PRIVATE -g4 -gsource-map)
target_link_options(game PRIVATE -g4)
add_definitions("-s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\", \"UTF8ToString\"]'")
add_definitions("-s EXTRA_EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\", \"UTF8ToString\"]'")
add_link_options("--source-map-base=http://localhost:4000")
add_definitions("-s EXPORTED_FUNCTIONS='[\"_testInt\"]'")
#add_definitions("-s WASM=1")
ADD_DEFINITIONS("-s NO_EXIT_RUNTIME=1")
set_target_properties(game PROPERTIES LINK_FLAGS "-s EXTRA_EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString']")

set(CMAKE_EXECUTABLE_SUFFIX ".html")
