# fable_anniversary - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(fable_anniversary LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/AnniversaryRuntime.cmake)

# Sources
set(FABLEANNIVERSARY_SOURCES
    src/main.cpp
    src/startup_callbacks.cpp
    src/content_compat.cpp
    src/fiber_compat.cpp
    src/audio_compat.cpp
)

if(WIN32)
    add_executable(fable_anniversary WIN32 ${FABLEANNIVERSARY_SOURCES})
else()
    add_executable(fable_anniversary ${FABLEANNIVERSARY_SOURCES})
endif()

rexglue_setup_target(fable_anniversary GPU_PLUGINS xenos)

if(WIN32)
    target_link_options(fable_anniversary PRIVATE LINKER:/DEBUG:FULL LINKER:/INCREMENTAL:NO)
endif()
