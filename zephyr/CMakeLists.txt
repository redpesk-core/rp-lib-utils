if(CONFIG_AFB_BINDER)
    set(WITH_ZEPHYR 1)

    set(NO_STATIC_LIB 0)
    set(NO_SHARED_LIB 1)
    set(WITHOUT_JSON_C 1)
    set(WITHOUT_JSON_NLOHMANN 1)
    set(WITHOUT_SYSTEMD 1)
    set(WITHOUT_CURL 1)
    set(WITHOUT_YAML 1)
    set(WITHOUT_PROCFS 1)

    add_subdirectory(.. rp-lib-utils)
endif()