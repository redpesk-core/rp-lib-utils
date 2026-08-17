
pkg_check_modules(CHECK check IMPORTED_TARGET)

if(NOT NO_STATIC_LIB)
	set(MODE static)
else()
	set(MODE shared)
endif()

if(TARGET rp-utils-yaml-${MODE})
	add_executable(yaml2json yaml2json.c)
	target_link_libraries(yaml2json rp-utils-yaml-${MODE})
	target_include_directories(yaml2json PRIVATE ../src)
	install(TARGETS yaml2json)
	message("Adding tool yaml2json")
else()
	message("NO tool yaml2json")
endif()

