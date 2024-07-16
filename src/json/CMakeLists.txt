
# module json-c
message(STATUS "1. WITH_JSON_C=${WITH_JSON_C}")
optmod(json-c>=0.13 WITH_JSON_C)
message(STATUS "2. WITH_JSON_C=${WITH_JSON_C}")
if(WITH_JSON_C)
	addsrc(rp-jconf.c rp-jsonc-expand.c rp-jsonc-path.c rp-jsonc.c)
	addhdr(rp-jconf.h rp-jsonc-expand.h rp-jsonc-path.h rp-jsonc.h)
	adddoc(rp-jsonc.md)
endif()

# our files without json-c
addsrc(rp-jsonstr.c)
addhdr(rp-jsonstr.h)

# for nlohmann/json c++
if(NOT WITHOUT_JSON_NLOHMANN)
	addhdr(json.hpp)
endif()
