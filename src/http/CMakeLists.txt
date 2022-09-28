# need module curl
reqmod(libcurl)

# our files
addsrc(rp-escape.c rp-curl.c)
addhdr(rp-escape.h rp-curl.h)
