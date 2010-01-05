
AM_CFLAGS = -fPIC 

libexecdir=$(libdir)

libexec_PROGRAMS = libdistdb.so

libdistdb_so_SOURCES = version.c client.c

libdistdb_so_LDFLAGS = -pie -Wl,--export-dynamic 
