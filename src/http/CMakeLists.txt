# need module curl
optmod(libcurl WITH_CURL)

# our files
if(WITH_CURL)
	addsrc(rp-curl.c)
	addhdr(rp-curl.h)
endif()

addsrc(rp-escape.c)
addhdr(rp-escape.h)
