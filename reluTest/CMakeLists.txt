if (CMAKE_TOOLCHAIN_FILE)
    set(LIBRARY_OUTPUT_PATH_ROOT ${CMAKE_BINARY_DIR} CACHE PATH "root for library output, set this to change where android libs are compiled to")
    # get absolute path, but get_filename_component ABSOLUTE only refer with source dir, so find_file here :(
    get_filename_component(CMAKE_TOOLCHAIN_FILE_NAME ${CMAKE_TOOLCHAIN_FILE} NAME)
    find_file(CMAKE_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_FILE_NAME} PATHS ${CMAKE_SOURCE_DIR} NO_DEFAULT_PATH)
    message(STATUS "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
elseif (ANDROID)
    message(FATAL_ERROR "Please specify a toolchain file.")
endif ()

cmake_minimum_required(VERSION 3.5)
project(getCPUid)

set(CMAKE_CXX_STANDARD 11)

# generate compile_commands.json for function jumps
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#include_directories(${PROJECT_SOURCE_DIR}/src)

# Only usefull in gcc
if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif ()

add_definitions(-fPIC)
add_definitions(-Ofast)
add_definitions(-ffast-math)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE -pie")

FILE(GLOB AArch32_SRC ${CMAKE_CURRENT_LIST_DIR}/arm32/*.s ${CMAKE_CURRENT_LIST_DIR}/arm32/*.S)
FILE(GLOB AArch64_SRC ${CMAKE_CURRENT_LIST_DIR}/arm64/*.s ${CMAKE_CURRENT_LIST_DIR}/arm64/*.S)

# set(OBJECTS_TO_LINK "")
set(ARM_SRCS "")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^armv7" OR ARCHS MATCHES "^armv7(;armv7s)?")
    message(STATUS "Enabling AArch32 Assemblies")
    set(ARM_SRCS ${AArch32_SRC})
    # add_library(ARM OBJECT ${AArch32_SRC})
    # list(APPEND OBJECTS_TO_LINK $<TARGET_OBJECTS:ARM>)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64" OR ARCHS STREQUAL "arm64")
    message(STATUS "Enabling AArch64 Assemblies")
    set(ARM_SRCS ${AArch64_SRC})
    # add_library(ARM OBJECT ${AArch64_SRC})
    # list(APPEND OBJECTS_TO_LINK $<TARGET_OBJECTS:ARM>)
endif()

message("******** ARM_SRCS: ${ARM_SRCS}")

# add_executable(assemblyReluTest assemblyReluTest.cpp ${OBJECTS_TO_LINK})
add_executable(assemblyReluTest ${ARM_SRCS} assemblyReluTest.cpp)



