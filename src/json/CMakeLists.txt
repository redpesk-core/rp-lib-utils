
# module json-c
optmod(json-c>=0.13 WITH_JSON_C)

message(STATUS "json. WITH_JSON_C=${WITH_JSON_C}")
message(STATUS "json. WITHOUT_JSON_NLOHMANN=${WITHOUT_JSON_NLOHMANN}")

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
