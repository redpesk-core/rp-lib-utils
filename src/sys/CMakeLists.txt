optmod(libsystemd>=222 WITH_SYSTEMD)

if(WITH_SYSTEMD)
	addsrc(rp-systemd.c)
	addhdr(rp-systemd.h)
endif()

addsrc(
	rp-verbose.c
	rp-socket.c
)
addhdr(
	rp-verbose.h
	rp-socket.h
)

