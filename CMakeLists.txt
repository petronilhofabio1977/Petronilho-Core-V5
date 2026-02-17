cmake_minimum_required(VERSION 3.20)
project(SUPER_CORE_RUNTIME LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
include_directories(${PROJECT_SOURCE_DIR})

# Forçando o Petronilho a ser conservador:
# -mno-avx desativa qualquer tentativa do compilador de usar AVX
add_compile_options(-O2 -march=westmere -mno-avx -mno-avx2 -mno-bmi -mno-bmi2 -pthread -fno-exceptions)

add_subdirectory(core)
add_subdirectory(priority_wing)
add_subdirectory(geometric_wing)
add_subdirectory(relational)

add_executable(test_network_ingest perf/test_network_ingest.cpp)
target_link_libraries(test_network_ingest PRIVATE core_static)
