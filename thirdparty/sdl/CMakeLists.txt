
set(lib_name sdl)
set(target_name ${lib_name})
project(${lib_name})

include(AXPlatform)

add_library(${target_name} STATIC IMPORTED GLOBAL)

set(SDL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE STRING "SDL include dir" FORCE)

set_target_properties(${target_name} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
set_target_properties(${target_name} PROPERTIES
  IMPORTED_LOCATION "${platform_spec_path}/libSDL2.a"
)
