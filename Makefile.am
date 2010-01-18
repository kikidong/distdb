
SUBDIRS = include src init lib db doc

EXTRA_DIST = nodes.db.in Doxygen.in distdb.conf.in

bin_PROGRAMS = distdb

distdb_SOURCES  = init/main.c init/inifile.c init/gennodes.c init/prase.c

distdb_SOURCES += src/global_var.c src/rpc_op.c src/masterloop.c src/database.c

distdb_DEPENDENCES = init/inifile.h init/prase.h

distdb_LDFLAGS  = -Wl,--export-dynamic -as-need -Wl,-rpath,$(libdir)/distdb,-rpath,$(bindir)/,-rpath,./

sysconf_DATA = distdb.conf
localstate_DATA = nodes.db

html-local:
	cd doc && ${MAKE} $@