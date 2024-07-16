#/bin/bash

export NO_STATIC_LIB=OFF
export NO_SHARED_LIB=ON
export WITHOUT_JSON_C=ON
export WITHOUT_JSON_NLOHMANN=ON
export WITHOUT_SYSTEMD=ON
export WITHOUT_CURL=ON
export WITHOUT_YAML=ON
export WITHOUT_PROCFS=ON

export bd=build-zephyr
$(dirname $0)/mkbuild.sh "$@"
