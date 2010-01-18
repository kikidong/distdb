
AM_CFLAGS = -fPIC 

EXTRA_DIST = version.c client.c java.c

libexecdir=$(libdir)

libexec_PROGRAMS = libdistdb.so

libdistdb_so_SOURCES = version.c client.c

libdistdb_so_LDFLAGS = -pie -Wl,--export-dynamic 
