add_library(plugin SHARED plugin.cpp plugin.h)
set(CMAKE_CXX_STANDARD 17)
if(WIN32)
    target_compile_definitions(plugin PRIVATE PLUGIN_EXPORTS)
endif()

set_target_properties(plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/)