# need module json-c
reqmod(json-c)

# our files
addsrc(rp-jsonc-expand.c rp-jsonc-locator.c rp-jsonc-path.c rp-jsonstr.c rp-jsonc.c)
addhdr(rp-jsonc-expand.h rp-jsonc-locator.h rp-jsonc-path.h rp-jsonstr.h rp-jsonc.h)
adddoc(rp-jsonc.md)

# for nlohmann/json c++
addhdr(json.hpp)
