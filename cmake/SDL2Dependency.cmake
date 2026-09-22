# Reuse the existing engine's SDL2 import library without depending on the engine.
# Linux/WSL always uses Linux-built libraries, never Windows .lib/.dll files.
if(VISIONLAB_THIRD_PARTY_ROOT AND WIN32 AND MSVC)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "The shared SDL2 layout supports x64 only.")
    endif()
    set(sdl_root "${VISIONLAB_THIRD_PARTY_ROOT}/SDL")
    foreach(required include/SDL.h lib/x64/SDL2.lib lib/x64/SDL2.dll)
        if(NOT EXISTS "${sdl_root}/${required}")
            message(FATAL_ERROR "Missing ${sdl_root}/${required}; correct VISIONLAB_THIRD_PARTY_ROOT or use an SDL2 CMake package.")
        endif()
    endforeach()
    add_library(SDL2::SDL2 SHARED IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
        IMPORTED_IMPLIB "${sdl_root}/lib/x64/SDL2.lib"
        IMPORTED_LOCATION "${sdl_root}/lib/x64/SDL2.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${sdl_root}/include")
else()
    find_package(SDL2 2 REQUIRED CONFIG)
    if(NOT TARGET SDL2::SDL2)
        message(FATAL_ERROR "SDL2 package must export SDL2::SDL2.")
    endif()
endif()
