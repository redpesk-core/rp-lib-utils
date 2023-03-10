# add our requirements
reqmod(libsystemd>=222)
# add our files
addsrc(
	rp-base64.c
	rp-enum-map.c
	rp-expand-vars.c
	rp-file.c
	rp-path-search.c
	rp-pearson.c
	rp-read-fd-name.c
	rp-socket.c
	rp-uuid.c
	sha1.c
)
addhdr(
	rp-base64.h
	rp-enum-map.h
	rp-expand-vars.h
	rp-file.h
	rp-path-search.h
	rp-pearson.h
	rp-read-fd-name.h
	rp-socket.h
	rp-uuid.h
	sha1.h
)
