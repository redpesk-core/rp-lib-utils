if(CONFIG_AFB_BINDER)
    set(WITH_ZEPHYR 1)

    set(NO_STATIC_LIB OFF)
    set(NO_SHARED_LIB ON)
    set(WITHOUT_JSON_C ON)
    set(WITHOUT_JSON_NLOHMANN ON)
    set(WITHOUT_SYSTEMD ON)
    set(WITHOUT_CURL ON)
    set(WITHOUT_YAML ON)
    set(WITHOUT_PROCFS ON)
    set(WITH_DIRENT OFF)

    add_definitions(-DWITH_DIRENT=OFF)
    
    add_subdirectory(.. rp-lib-utils)
endif()