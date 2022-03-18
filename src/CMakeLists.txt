# current directory prefix
set(dirprefix "")

# global properties for handling headers, modules and private dependencies
set_property(GLOBAL PROPERTY headers)
set_property(GLOBAL PROPERTY modules)
set_property(GLOBAL PROPERTY privates)

# the static library target
add_library(rp-utils-static STATIC)
set_target_properties(rp-utils-static PROPERTIES
	OUTPUT_NAME rp-utils
	VERSION "${LIBRP_UTILS_VERSION}")

# the shared library target
add_library(rp-utils-shared SHARED)
set_target_properties(rp-utils-shared PROPERTIES
	OUTPUT_NAME rp-utils
	VERSION "${LIBRP_UTILS_VERSION}"
	SOVERSION 0)

# macro for adding a source file
macro(addsrc)
	target_sources(rp-utils-static PUBLIC ${ARGV})
	target_sources(rp-utils-shared PUBLIC ${ARGV})
endmacro(addsrc)

# macro for adding a header file
macro(addhdr)
	foreach(hdr ${ARGV})
		set_property(GLOBAL APPEND PROPERTY headers "${dirprefix}/${hdr}")
	endforeach(hdr)
endmacro(addhdr)

# macro for requiring modules
macro(reqmod)
	pkg_check_modules(mods REQUIRED ${ARGV})
	target_compile_options(rp-utils-static PUBLIC ${mods_CFLAG})
	target_compile_options(rp-utils-shared PUBLIC ${mods_CFLAG})
	target_link_options(rp-utils-shared PUBLIC ${mods_LDFLAGS})
	set_property(GLOBAL APPEND PROPERTY modules ${ARGV})
	set_property(GLOBAL APPEND PROPERTY privates ${mods_LDFLAGS})
endmacro(reqmod)

# macro for adding documentation
macro(adddoc)
endmacro(adddoc)

# macro for adding directories
macro(adddir dir)
	set(dirprefix ${dir})
	add_subdirectory(${dir})
endmacro(adddir)

# add the sub directories
adddir(http)
adddir(json)
adddir(misc)
adddir(sys)

# install the libraries
install(TARGETS rp-utils-static rp-utils-shared)

# install the headers
get_property(headers GLOBAL PROPERTY headers)
install(FILES ${headers} DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}/rp-utils")

# create and install the pkgconfig (PC) file
get_property(modules GLOBAL PROPERTY modules)
list(JOIN modules " " modules)
get_property(privates GLOBAL PROPERTY privates)
list(JOIN privates " " privates)
configure_file(librp-utils.pc.in librp-utils.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/librp-utils.pc DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")
