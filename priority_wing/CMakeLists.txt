# priority_wing/CMakeLists.txt
# Layer: L1 - depende de core_static (L0)
# Governanca: priority_wing pode incluir core/

add_library(priority_wing_interface INTERFACE)

target_include_directories(priority_wing_interface
    INTERFACE ${CMAKE_SOURCE_DIR}
)

target_link_libraries(priority_wing_interface
    INTERFACE core_static
)
