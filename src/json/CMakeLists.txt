# need module json-c
reqmod(json-c)

# our files
addsrc(rp-json-expand.c rp-json-locator.c rp-json-path.c rp-jsonstr.c rp-json-wrap.c)
addhdr(rp-json-expand.h rp-json-locator.h rp-json-path.h rp-jsonstr.h rp-json-wrap.h)
adddoc(rp-wrap-json.md)

# for nlohmann/json c++
addhdr(json.hpp)