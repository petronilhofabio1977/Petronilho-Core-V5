add_library(core_static STATIC 
    sys/sys_core.cpp 
    platform/vcore_sse4.cpp
)

target_compile_options(core_static PRIVATE -msse4.2 -march=westmere)
