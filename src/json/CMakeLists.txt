# need module json-c
reqmod(json-c>=0.13)

# our files
addsrc(rp-jconf.c rp-jsonc-expand.c rp-jsonc-path.c rp-jsonstr.c rp-jsonc.c)
addhdr(rp-jconf.h rp-jsonc-expand.h rp-jsonc-path.h rp-jsonstr.h rp-jsonc.h)
adddoc(rp-jsonc.md)

# for nlohmann/json c++
addhdr(json.hpp)
