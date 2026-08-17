rp-lib-utils
============

This project provides useful C/C++ routines used by many
projects developped by IoT.bzh company since 2015.

It is available with the liberal license MIT.

It also includes open source code from other origin
with liberal license (see Third-Party below)

This project is available in github at
https://github.com/redpesk-core/rp-lib-utils.

Bug reports must be done through creation of issues.

Contributions are welcome through creation of pull request.

Produced libraries
------------------

Since version 0.2.0, rp-lib-utils provides many small
libraries either shared or static:

- librp-utils-core: no dependency (or libuuid if available)
    - HTTP escaping (rp-escape)
    - JSON string formatting (rp-jsonstr)
    - Base64 codec (rp-base64)
    - Text <-> enum conversion (rp-enum-map)
    - Expansion of variables (rp-expand-vars)
    - Pearson hash (rp-pearson)
    - UUID (rp-uuid)
    - Conversion of text to integers (rp-str2int)
    - SHA1 (sha1)
    - Logging (rp-verbose)

- librp-utils-file: depends on POSIX files
    - Whole file read/write (rp-file)
    - File search or enumerate (rp-path-search)
    - iLocate executable (rp-whichprog)

- librp-utils-socket: depends on POSIX sockets
    - Opening using scheme (rp-socket)
    - Systemd socket retrieving (rp-systemd)

- librp-utils-json-c: depends on libjson-c
    - Get values with defaults (rp-jconf)
    - Expansion of JSON (rp-jsonc-expand)
    - Default JSON expansion(rp-jsonc-default-expand)
    - Path of an object (rp-jsonc-path)
    - JSON-C utilities (rp-jsonc)

- librp-utils-yaml: depends on libjson-c and libyaml
    - YAML to json-c (rp-yaml)

- librp-utils-curl: depends on curl
    - CURL helpers (rp-curl)

These libraries are made available through pkg-config PC files
(`librp-utils-{core,curl,file,json,socket,yaml}{,-static}.pc`).

So as an example, a program using librp-utils-core-static could
be compiled like this:

```
cc $(pkg-config --libs --cflags librp-utils-core-static) my-prog.c
```


Content
-------

Sources are in directories:

```
directory    : content
-------------+-------------------------------
src/http     : HTTP and curl
src/json     : json handling
src/misc     : miscellanous
src/sys      : system wrapping
src/sandbox  : staging array containing code no integrated
test         : some tests
tools        : some tools
zephyr       : Zephyr configuration
```

The generation is made by the dense CMake script
`src/CMakeLists.txt`


Third-Party
-----------

src/json/json.hpp: The JSON C++ code of Niels Lohmann
src/misc/sha1.c: SHA1 computation of Steve Reid (modified)

