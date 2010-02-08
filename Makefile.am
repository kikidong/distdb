
AM_CFLAGS = -fPIC -fpic @GLIB_CFLAGS@ 


SUBDIRS = include src init db doc

EXTRA_DIST = nodes.db.in Doxygen.in distdb.conf.in README.zh_CN

bin_PROGRAMS = distdb

distdb_SOURCES  = init/main.c init/inifile.c init/prase.c
distdb_SOURCES += src/global_var.c src/masterloop.c src/database.c 
distdb_SOURCES += src/nodes_op.c src/version.c src/easyinet.c src/distdb.c

distdb_DEPENDENCES = init/inifile.h init/prase.h include/*

distdb_LDFLAGS  = @GLIB_LIBS@ -pie -Wl,--export-dynamic -as-need -Wl,-rpath,$(libdir)/distdb,-rpath,$(bindir)/,-rpath,./

sysconf_DATA = distdb.conf
localstate_DATA = nodes.db

html-local:
	cd doc && ${MAKE} $@ 
