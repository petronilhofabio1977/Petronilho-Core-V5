cmake_minimum_required(VERSION 3.20)
project(SUPER_CORE_RUNTIME LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${PROJECT_SOURCE_DIR})

add_compile_options(-fno-exceptions -fno-rtti -O3 -march=westmere -pthread)

# Flags de Linkagem Estática para Windows
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
endif()

add_subdirectory(core)
add_subdirectory(priority_wing)
add_subdirectory(geometric_wing)
add_subdirectory(relational)

add_executable(super_core_final perf/super_core_final.cpp)
target_link_libraries(super_core_final PRIVATE core_static)
