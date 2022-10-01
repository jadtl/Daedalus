add_library(Engine INTERFACE)
add_library(Daedalus::Engine ALIAS Engine)
set_target_properties(Engine PROPERTIES OUTPUT_NAME "Daedalus")
target_include_directories(Engine INTERFACE src/)

target_compile_definitions(Engine 
    INTERFACE
    #PRIVATE 
    #DDLS_EXPORT
    #PUBLIC
    ## Version
    #DDLS_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
    #DDLS_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
    #DDLS_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}
    # Debug
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:DEBUG DDLS_DEBUG>
)