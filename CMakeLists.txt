cmake_minimum_required(VERSION 3.16)
project(rlsnake C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

set(RAYLIB_VERSION 5.5)

# raylib build options (must be set before the subdirectory is added)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

if(EMSCRIPTEN)
    set(PLATFORM Web CACHE STRING "" FORCE)
    set(OPENGL_VERSION "ES 2.0" CACHE STRING "" FORCE)
endif()

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG ${RAYLIB_VERSION}
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")

add_executable(rlsnake ${SOURCES})
target_link_libraries(rlsnake PRIVATE raylib)
target_include_directories(rlsnake PRIVATE "${CMAKE_SOURCE_DIR}/src")

# Anything in assets/ is bundled into the web build and copied next to the
# native binary. Safe to leave empty.
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

if(EMSCRIPTEN)
    # gh-pages serves <root>/index.html, so name the generated page that.
    set_target_properties(rlsnake PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
    )

    # raylib defines this too, but declare it locally so src/ never depends on
    # how raylib chose to scope it.
    target_compile_definitions(rlsnake PRIVATE PLATFORM_WEB)

    target_link_options(rlsnake PRIVATE
        "-sUSE_GLFW=3"
        "-sASYNCIFY"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sFORCE_FILESYSTEM=1"
        "-sGL_ENABLE_GET_PROC_ADDRESS=1"
        "-sSTACK_SIZE=1mb"
        "--shell-file" "${CMAKE_SOURCE_DIR}/src/shell.html"
    )

    if(EXISTS "${ASSETS_DIR}")
        target_link_options(rlsnake PRIVATE "--preload-file" "${ASSETS_DIR}@/assets")
    endif()
else()
    if(EXISTS "${ASSETS_DIR}")
        add_custom_command(TARGET rlsnake POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${ASSETS_DIR}" "$<TARGET_FILE_DIR:rlsnake>/assets"
        )
    endif()

    if(APPLE)
        target_link_libraries(rlsnake PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
    endif()
endif()
