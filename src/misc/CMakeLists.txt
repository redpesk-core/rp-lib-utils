# add our files

message(STATUS "misc. WITHOUT_PROCFS=${WITHOUT_PROCFS}")

addsrc(
	rp-base64.c
	rp-enum-map.c
	rp-expand-vars.c
	rp-file.c
	rp-path-search.c
	rp-pearson.c
	rp-uuid.c
	rp-whichprog.c
	sha1.c
)
addhdr(
	rp-base64.h
	rp-enum-map.h
	rp-expand-vars.h
	rp-file.h
	rp-path-search.h
	rp-pearson.h
	rp-uuid.h
	rp-whichprog.h
	sha1.h
)

if(NOT WITHOUT_PROCFS)
	addsrc(rp-read-fd-name.c)
	addhdr(rp-read-fd-name.c)
endif()
