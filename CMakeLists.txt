cmake_minimum_required(VERSION 3.10)
project(tinybit C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- TinyBit library (interface lib from subdir) ---
add_subdirectory(src/tinybit)

# --- Find SDL2 and SDL2_image ---
if(WIN32)
    # On Windows, expect SDL2 paths via CMAKE_PREFIX_PATH or environment
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
endif()

# --- Main executable ---
add_executable(tinybit
    src/main.c
    src/platform.c
    src/cartridge_io.c
    src/games.c
)

target_include_directories(tinybit PRIVATE src/)
target_link_libraries(tinybit PRIVATE tinybit_lib)

if(WIN32)
    target_include_directories(tinybit PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
    target_link_libraries(tinybit PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_image::SDL2_image)
else()
    target_include_directories(tinybit PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
    target_link_libraries(tinybit PRIVATE ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} m)
endif()

# --- Copy assets to build directory ---
add_custom_command(TARGET tinybit POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:tinybit>/assets
)

# --- Install ---
install(TARGETS tinybit DESTINATION bin)
install(DIRECTORY assets/ DESTINATION share/tinybit)
