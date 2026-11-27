# The Mac build: the Mac-only code of 1.14d, compiled, not linked (included from
# CMakeLists.txt when the target is Apple; toolchain cmake/mac-i386-clang425.cmake).
#
# d2_mac_objs  StormMac, the Mac Storm project (Source/StormMac/), and the game's Mac-only
#              module's Mac shell (Source/MacSpecific/Source/MacShell/). CD2Textures, the other
#              MacSpecific folder, is the D2OpenGL texture base on Win32/WGL, built by the
#              Windows target d2opengl_objs. The 1.14d Mac flags:
#              -Os -fstack-protector, libc++, Pascal string literals (Xcode's
#              default; "\p" literals sit in __TEXT,__const).
#
# Nothing of the Windows tree is built here; the shared code is not yet Mac-clean.

file(GLOB_RECURSE D2_MAC_SOURCES CONFIGURE_DEPENDS LIST_DIRECTORIES false
    "${CMAKE_SOURCE_DIR}/Source/StormMac/*.cpp"
    "${CMAKE_SOURCE_DIR}/Source/StormMac/*.CPP"
    "${CMAKE_SOURCE_DIR}/Source/StormMac/*.mm"
    "${CMAKE_SOURCE_DIR}/Source/D2gfx/Mac/*.cpp"
    "${CMAKE_SOURCE_DIR}/Source/MacSpecific/Source/MacShell/*.cpp")
list(REMOVE_DUPLICATES D2_MAC_SOURCES)
list(SORT D2_MAC_SOURCES)
# SRegMac.mm is Objective-C++; the project enables C and C++ only.
set_source_files_properties(${D2_MAC_SOURCES} PROPERTIES LANGUAGE CXX)
foreach(_src ${D2_MAC_SOURCES})
    if(_src MATCHES "\\.mm$")
        set_source_files_properties(${_src} PROPERTIES COMPILE_FLAGS "-x objective-c++")
    endif()
endforeach()

add_library(d2_mac_objs OBJECT ${D2_MAC_SOURCES})
target_include_directories(d2_mac_objs PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/Source/D2Headers")
# The target axes (Source/D2Headers/D2Arch.h): Mac OS X, i386, the client.
target_compile_definitions(d2_mac_objs PRIVATE D2_OS_MAC=1 D2_ARCH_X86=1 D2_TARGET_CLIENT=1)
target_compile_options(d2_mac_objs PRIVATE
    -Os -fstack-protector -fpascal-strings
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++11 -stdlib=libc++>
    -Werror=implicit-function-declaration)
