# fable_anniversary - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(fable_anniversary LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(generated/rexglue.cmake)

# Sources
set(FABLEANNIVERSARY_SOURCES
    src/main.cpp
)

if(WIN32)
    add_executable(fable_anniversary WIN32 ${FABLEANNIVERSARY_SOURCES})
else()
    add_executable(fable_anniversary ${FABLEANNIVERSARY_SOURCES})
endif()

rexglue_setup_target(fable_anniversary)
