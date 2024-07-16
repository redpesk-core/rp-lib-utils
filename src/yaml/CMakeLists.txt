# need module json-c
optmod(json-c>=0.13 WITH_JSON_C)
optmod(yaml-0.1     WITH_YAML)

# add our files
if(WITH_YAML AND WITH_JSON_C)
	addsrc(rp-yaml.c)
	addhdr(rp-yaml.h)
endif()
