# need module json-c
optmod(json-c>=0.13 WITH_JSON_C)
optmod(yaml-0.1     WITH_YAML)

message(STATUS "yaml. WITH_JSON_C=${WITH_JSON_C}")
message(STATUS "yaml. WITH_YAML=${WITH_YAML}")

# add our files
if(WITH_YAML AND WITH_JSON_C)
	addsrc(rp-yaml.c)
	addhdr(rp-yaml.h)
endif()
