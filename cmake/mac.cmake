# The Mac build: the Mac-only code of 1.14d, compiled, not linked (included from
# CMakeLists.txt when the target is Apple; toolchain cmake/mac-i386-clang425.cmake).
#
# d2_mac_objs  Storm's Mac layer, 3rdparty/Storm/Source/Mac/. The 1.14d Mac flags:
#              -Os -fstack-protector, libc++, Pascal string literals (Xcode's
#              default; "\p" literals sit in __TEXT,__const).
#
# Nothing of the Windows tree is built here; the shared code is not yet Mac-clean.

file(GLOB_RECURSE D2_MAC_SOURCES CONFIGURE_DEPENDS LIST_DIRECTORIES false
    "${CMAKE_SOURCE_DIR}/3rdparty/Storm/Source/Mac/*.cpp")
list(SORT D2_MAC_SOURCES)

add_library(d2_mac_objs OBJECT ${D2_MAC_SOURCES})
target_include_directories(d2_mac_objs PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/Source/D2Headers")
target_compile_options(d2_mac_objs PRIVATE
    -Os -fstack-protector -fpascal-strings
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++11 -stdlib=libc++>
    -Werror=implicit-function-declaration)
