optmod(libsystemd>=222 WITH_SYSTEMD)

addsrc(
	rp-verbose.c
	rp-socket.c
)
addhdr(
	rp-verbose.h
	rp-socket.h
)

if(WITH_SYSTEMD)
	addsrc(
		rp-systemd.c
	)
	addhdr(
		rp-systemd.h
	)
endif()