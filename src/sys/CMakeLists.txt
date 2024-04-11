optmod(libsystemd>=222 WITH_SYSTEMD)

addsrc(
	rp-verbose.c
	rp-socket.c
	rp-systemd.c
)
addhdr(
	rp-verbose.h
	rp-socket.h
	rp-systemd.h
)

