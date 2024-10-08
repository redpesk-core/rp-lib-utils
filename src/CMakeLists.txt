
# ensure relocatable
include_directories(http json misc sys yaml)
add_compile_options(-fPIC -ffunction-sections -fdata-sections)
add_link_options(-Wl,--as-needed -Wl,--gc-sections)

# current directory prefix
set(dirprefix "")

# global properties for handling headers, modules and private dependencies
set_property(GLOBAL PROPERTY headers)
set_property(GLOBAL PROPERTY modules)
set_property(GLOBAL PROPERTY privates)

# the static library target
if(NOT NO_STATIC_LIB)
	add_library(rp-utils-static STATIC)
	set_target_properties(rp-utils-static PROPERTIES
		OUTPUT_NAME rp-utils
		VERSION "${LIBRP_UTILS_VERSION}")
endif()

# the shared library target
if(NOT NO_SHARED_LIB)
	add_library(rp-utils-shared SHARED)
	set_target_properties(rp-utils-shared PROPERTIES
		OUTPUT_NAME rp-utils
		VERSION "${LIBRP_UTILS_VERSION}"
		SOVERSION 0)
endif()

# macro for adding a source file
macro(addsrc)
	if(NOT NO_STATIC_LIB)
		target_sources(rp-utils-static PUBLIC ${ARGV})
	endif()
	if(NOT NO_SHARED_LIB)
		target_sources(rp-utils-shared PUBLIC ${ARGV})
	endif()
endmacro(addsrc)

# macro for adding a header file
macro(addhdr)
	foreach(hdr ${ARGV})
		set_property(GLOBAL APPEND PROPERTY headers "${dirprefix}/${hdr}")
	endforeach(hdr)
endmacro(addhdr)

# function for adding properties
function(addprop name values)
	get_property(lv GLOBAL PROPERTY ${name})
	list(APPEND lv ${values})
	list(REMOVE_DUPLICATES lv)
	set_property(GLOBAL PROPERTY ${name} ${lv})
endfunction(addprop)

# function for requiring modules
function(reqmod)
	pkg_check_modules(mods REQUIRED ${ARGV})
	if(NOT NO_STATIC_LIB)
		target_compile_options(rp-utils-static PUBLIC ${mods_CFLAG})
	endif()
	if(NOT NO_SHARED_LIB)
		target_compile_options(rp-utils-shared PUBLIC ${mods_CFLAG})
		target_link_libraries(rp-utils-shared PUBLIC ${mods_LDFLAGS})
	endif()
	addprop(modules "${ARGV}")
	addprop(privates "${mods_LDFLAGS}")
endfunction(reqmod)

# function for optional modules
function(optmod name cvar)
	string(REPLACE "WITH_" "" notcvar ${cvar})
	set(notcvar "WITHOUT_${notcvar}")
	get_property(defined GLOBAL PROPERTY optmod_${cvar}_checked SET)
	if(defined)
		get_property(mod_FOUND GLOBAL PROPERTY optmod_${cvar}_checked)
	else()
		set(mod_FOUND NO)
		if(NOT ${notcvar} AND ( ${cvar} OR "<${${cvar}}>" STREQUAL "<>" ))
			pkg_check_modules(mod ${name})
		endif()
		if(mod_FOUND)
			if(NOT NO_STATIC_LIB)
				target_compile_definitions(rp-utils-static PUBLIC ${cvar}=1 ${notcvar}=0)
				target_compile_options(rp-utils-static PUBLIC ${mod_CFLAG})
			endif()
			if(NOT NO_SHARED_LIB)
				target_compile_definitions(rp-utils-shared PUBLIC ${cvar}=1 ${notcvar}=0)
				target_compile_options(rp-utils-shared PUBLIC ${mod_CFLAG})
				target_link_libraries(rp-utils-shared PUBLIC ${mod_LDFLAGS})
			endif()
			addprop(modules "${name}")
			addprop(privates "${mod_LDFLAGS}")
			message(STATUS "External package ${name} is used")
		else()
			message(STATUS "External package ${name} not used")
			if(NOT NO_STATIC_LIB)
				target_compile_definitions(rp-utils-static PUBLIC ${cvar}=0 ${notcvar}=1)
			endif()
			if(NOT NO_SHARED_LIB)
				target_compile_definitions(rp-utils-shared PUBLIC ${cvar}=0 ${notcvar}=1)
			endif()
		endif()
		set_property(GLOBAL PROPERTY optmod_${cvar}_checked ${mod_FOUND})
	endif()
	if(mod_FOUND)
		set(${cvar} ON PARENT_SCOPE)
		set(${notcvar} OFF PARENT_SCOPE)
	else()
		set(${cvar} OFF PARENT_SCOPE)
		set(${notcvar} ON PARENT_SCOPE)
	endif()
endfunction(optmod)

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
adddir(yaml)

# install the headers
get_property(headers GLOBAL PROPERTY headers)
message(STATUS "Installing headers ${headers}")
install(FILES ${headers} DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}/rp-utils")

# create and install the pkgconfig (PC) file
get_property(modules GLOBAL PROPERTY modules)
list(JOIN modules ", " modules)
string(REGEX REPLACE "([=<>])" " \\1 " modules "${modules}")
string(REGEX REPLACE "<  =" "<=" modules "${modules}")
string(REGEX REPLACE ">  =" ">=" modules "${modules}")
get_property(privates GLOBAL PROPERTY privates)
list(JOIN privates " " privates)
message(STATUS "Computed module list ${modules}")
message(STATUS "Computed private list ${privates}")

# the static library target
if(NOT NO_STATIC_LIB)
	configure_file(librp-utils-static.pc.in librp-utils-static.pc @ONLY)
	install(TARGETS rp-utils-static)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/librp-utils-static.pc DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")
endif()

# the shared library target
if(NOT NO_SHARED_LIB)
	configure_file(librp-utils.pc.in librp-utils.pc @ONLY)
	install(TARGETS rp-utils-shared)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/librp-utils.pc DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")
endif()

