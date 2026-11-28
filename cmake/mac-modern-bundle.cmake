# Assembles "Diablo II.app" from the d2_modern executable (cmake -P, the d2_modern_app target in
# cmake/mac-modern.cmake): the Info.plist, SDL3 in Contents/Frameworks with the executable
# pointed at it through @rpath, an ad-hoc signature, and the checks that the bundle verifies
# and loads nothing from outside the system and itself.
#
# In:  D2_EXE (the linked executable), D2_BUNDLE (the .app to write), D2_SDL3 (the SDL3 dylib
#      linked against), D2_PLIST_IN (the Info.plist template), D2_BUNDLE_ID, D2_BUNDLE_VERSION,
#      D2_BUNDLE_BUILD.
# The game data is never part of it: the player's own MPQs are found at run time
# (Source/StormMac/Win32Posix/GameData.cpp).

foreach(_v D2_EXE D2_BUNDLE D2_SDL3 D2_PLIST_IN D2_BUNDLE_ID D2_BUNDLE_VERSION D2_BUNDLE_BUILD)
    if(NOT DEFINED ${_v})
        message(FATAL_ERROR "mac-modern-bundle: ${_v} is not set")
    endif()
endforeach()

function(d2_run)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE _rc OUTPUT_VARIABLE _out ERROR_VARIABLE _err)
    if(NOT _rc EQUAL 0)
        string(REPLACE ";" " " _cmd "${ARGN}")
        message(FATAL_ERROR "mac-modern-bundle: `${_cmd}` failed (${_rc}):\n${_out}${_err}")
    endif()
    set(D2_RUN_OUT "${_out}${_err}" PARENT_SCOPE)
endfunction()

set(_contents "${D2_BUNDLE}/Contents")
set(_exe "${_contents}/MacOS/Diablo II")
file(REMOVE_RECURSE "${D2_BUNDLE}")
file(MAKE_DIRECTORY "${_contents}/MacOS" "${_contents}/Frameworks" "${_contents}/Resources")
configure_file("${D2_PLIST_IN}" "${_contents}/Info.plist" @ONLY)
file(WRITE "${_contents}/PkgInfo" "APPL????")
file(COPY_FILE "${D2_EXE}" "${_exe}")

# SDL3, under the name the executable links (libSDL3.0.dylib), real file not symlink.
file(REAL_PATH "${D2_SDL3}" _sdl_real)
d2_run(otool -D "${_sdl_real}")
string(REGEX MATCH "[^\n/]+\\.dylib\n?$" _sdl_leaf "${D2_RUN_OUT}")
string(STRIP "${_sdl_leaf}" _sdl_leaf)
set(_sdl "${_contents}/Frameworks/${_sdl_leaf}")
file(COPY_FILE "${_sdl_real}" "${_sdl}")
file(CHMOD "${_sdl}" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
d2_run(install_name_tool -id "@rpath/${_sdl_leaf}" "${_sdl}")

# The executable's reference to it, whatever path the build linked.
d2_run(otool -L "${_exe}")
string(REGEX MATCH "\t([^\n]*/${_sdl_leaf}) \\(" _m "${D2_RUN_OUT}")
if(NOT CMAKE_MATCH_1)
    message(FATAL_ERROR "mac-modern-bundle: ${_exe} does not link ${_sdl_leaf}:\n${D2_RUN_OUT}")
endif()
if(NOT CMAKE_MATCH_1 STREQUAL "@rpath/${_sdl_leaf}")
    d2_run(install_name_tool -change "${CMAKE_MATCH_1}" "@rpath/${_sdl_leaf}" "${_exe}")
endif()
# The build tree's rpaths (CMake's /opt/homebrew/lib) go: the bundle's SDL3 is the only one.
d2_run(otool -l "${_exe}")
string(REGEX MATCHALL "cmd LC_RPATH\n[^\n]*\n +path [^\n]+ \\(offset" _rpaths "${D2_RUN_OUT}")
foreach(_r IN LISTS _rpaths)
    string(REGEX REPLACE ".*path (.+) \\(offset$" "\\1" _r "${_r}")
    d2_run(install_name_tool -delete_rpath "${_r}" "${_exe}")
endforeach()
d2_run(install_name_tool -add_rpath "@executable_path/../Frameworks" "${_exe}")

# The ad-hoc signature (install_name_tool invalidated the linker's), then the checks.
d2_run(codesign --force --deep -s - "${D2_BUNDLE}")
d2_run(codesign --verify --deep --strict "${D2_BUNDLE}")
foreach(_bin "${_exe}" "${_sdl}")
    d2_run(otool -L "${_bin}")
    string(REGEX MATCHALL "\t[^\n(]+" _deps "${D2_RUN_OUT}")
    foreach(_dep IN LISTS _deps)
        string(STRIP "${_dep}" _dep)
        if(NOT _dep MATCHES "^(/System/Library/|/usr/lib/|@rpath/${_sdl_leaf}$)")
            message(FATAL_ERROR "mac-modern-bundle: ${_bin} loads ${_dep} from outside the bundle and the system")
        endif()
    endforeach()
endforeach()
message(STATUS "mac-modern-bundle: ${D2_BUNDLE} (ad-hoc signed, SDL3 ${_sdl_leaf} bundled)")
