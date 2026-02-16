# core/CMakeLists.txt
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 1. Driver SSE4 (O seu alvo atual)
add_library(vcore_sse4 OBJECT platform/vcore_sse4.cpp)
target_compile_options(vcore_sse4 PRIVATE -msse4.2 -O3 -fno-exceptions)

# 2. Driver AVX2 (Para máquinas modernas)
add_library(vcore_avx2 OBJECT platform/vcore_avx2.cpp)
target_compile_options(vcore_avx2 PRIVATE -mavx2 -mfma -O3 -fno-exceptions)

# 3. Âncora do Core
add_library(core_static STATIC sys/sys_core.cpp)
target_link_libraries(core_static PUBLIC vcore_sse4 vcore_avx2)
target_include_directories(core_static PUBLIC ${CMAKE_SOURCE_DIR})