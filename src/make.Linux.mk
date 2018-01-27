OS_DEFS += -DBOOST_NO_CXX11_SCOPED_ENUMS

    # Make some important things such as the global offset table read only as soon as
    # the dynamic linker is finished building it. This will prevent overwriting of addresses
    # which would later be jumped to.
    LDHARDENING+=-Wl,-z,relro -Wl,-z,now
