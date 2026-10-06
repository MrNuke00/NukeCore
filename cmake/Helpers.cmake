
# nukecore_get_cpp_sources(<out_var> <dir1> [<dir2> ...])
function(nukecore_get_cpp_sources OUTPUT_VAR)
    set(_files "")
    foreach(_dir IN LISTS ARGN)
        file(GLOB_RECURSE _found
            CONFIGURE_DEPENDS
            "${_dir}/*.hpp"
            "${_dir}/*.h"
            "${_dir}/*.cpp"
            "${_dir}/*.c"
        )
        list(APPEND _files ${_found})
    endforeach()
    set(${OUTPUT_VAR} "${_files}" PARENT_SCOPE)
endfunction()

