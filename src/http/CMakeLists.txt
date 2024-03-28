# need module curl
optmod(libcurl WITH_LIBCURL)

# our files
if(WITH_LIBCURL)
	addsrc(rp-escape.c rp-curl.c)
	addhdr(rp-escape.h rp-curl.h)
endif()
