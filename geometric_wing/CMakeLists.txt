# geometric_wing/CMakeLists.txt
# Layer: L1 - depende de core_static (L0)
# Governanca: geometric_wing pode incluir core/

add_library(geometric_wing_interface INTERFACE)

target_include_directories(geometric_wing_interface
    INTERFACE ${CMAKE_SOURCE_DIR}
)

target_link_libraries(geometric_wing_interface
    INTERFACE core_static
)