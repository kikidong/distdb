
AM_CFLAGS = -fPIC 

EXTRA_DIST = version.c client.c java.c

libexecdir=$(libdir)

libexec_PROGRAMS = libdistdb.so

libdistdb_so_SOURCES = version.c client.c

if BUILD_JAVA

libdistdb_so_SOURCES += java.c

libdistdb_so_DEPENDENCIES = $(top_builddir)/bindings/java/distdb.h

$(top_builddir)/bindings/java/distdb.h: $(top_srcdir)/bindings/java/distdb.java
	@ cd $(top_builddir)/bindings/java && ${MAKE} distdb.h

endif

libdistdb_so_LDFLAGS = -pie -Wl,--export-dynamic 
