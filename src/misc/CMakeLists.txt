# add our requirements
reqmod(libsystemd>=222)
# add our files
addsrc(
	rp-base64.c
	rp-expand-vars.c
	rp-file.c
	rp-pearson.c
	rp-socket.c
	rp-uuid.c
)
addhdr(
	rp-base64.h
	rp-expand-vars.h
	rp-file.h
	rp-pearson.h
	rp-socket.h
	rp-uuid.h
)
