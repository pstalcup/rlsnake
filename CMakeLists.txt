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

# --- cJSON (data files) ----------------------------------------------------
set(CJSON_VERSION v1.7.18)

set(ENABLE_CJSON_TEST OFF CACHE BOOL "" FORCE)
set(ENABLE_CJSON_UTILS OFF CACHE BOOL "" FORCE)
# raylib already registers a global "uninstall" target and cJSON's is not
# guarded by if(NOT TARGET uninstall) — leaving this on is a configure error.
set(ENABLE_CJSON_UNINSTALL OFF CACHE BOOL "" FORCE)
set(ENABLE_TARGET_EXPORT OFF CACHE BOOL "" FORCE)
set(ENABLE_CUSTOM_COMPILER_FLAGS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_AND_STATIC_LIBS OFF CACHE BOOL "" FORCE)
set(CJSON_OVERRIDE_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(CJSON_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    cjson
    GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
    GIT_TAG ${CJSON_VERSION}
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(cjson)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")

add_executable(rlsnake ${SOURCES})
target_link_libraries(rlsnake PRIVATE raylib cjson)
target_include_directories(rlsnake PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    # cJSON's target does not export its include dir when ENABLE_TARGET_EXPORT
    # is off, so point at the fetched source tree directly.
    "${cjson_SOURCE_DIR}"
)

# --- Assets ----------------------------------------------------------------
# assets/ is the single source of truth. On web it is baked into index.data by
# emscripten's preloader and mounted at /assets in the virtual filesystem; on
# desktop it is copied next to the binary and InitAssets() anchors the working
# directory there. So the same relative path works on both platforms:
#
#     LoadTexture("assets/images/snake.png");
#     LoadJsonAsset("assets/data/levels.json");
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

# CONFIGURE_DEPENDS re-runs this glob when files appear or disappear, so
# dropping in a new image does not require a manual cmake reconfigure.
file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS "${ASSETS_DIR}/*")
list(FILTER ASSET_FILES EXCLUDE REGEX "/\\.gitkeep$")

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

    # Skipped entirely when there is nothing to pack, which avoids emitting an
    # empty index.data that the page would still try to fetch.
    if(ASSET_FILES)
        target_link_options(rlsnake PRIVATE "--preload-file" "${ASSETS_DIR}@/assets")

        # --preload-file is opaque to CMake's dependency scanner: without this,
        # editing a PNG would not trigger a relink and index.data would go stale.
        set_property(TARGET rlsnake APPEND PROPERTY LINK_DEPENDS "${ASSET_FILES}")
    endif()
else()
    if(ASSET_FILES)
        add_custom_command(TARGET rlsnake POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${ASSETS_DIR}" "$<TARGET_FILE_DIR:rlsnake>/assets"
            COMMENT "Copying assets next to the binary"
        )
    endif()

    if(APPLE)
        target_link_libraries(rlsnake PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
    endif()
endif()
