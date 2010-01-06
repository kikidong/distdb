AM_CFLAGS = -fPIC -pie
AM_LDFLAGS = -Wl,--unresolved-symbols=ignore-all,--export-dynamic,-init,__init,-z,nodelete

libexecdir=$(libdir)/distdb

lib_LTLIBRARIES =
libexec_PROGRAMS = 

if WITH_SQLITE
libexec_PROGRAMS += sqlite.so

sqlite_so_SOURCES = sqlite.c
sqlite_so_LDADD = -lsqlite3 
#sqlite_so_DEPENDENCES = $(top_builddir)/distdb
endif

if WITH_MYSQL
libexec_PROGRAMS += mysql.so
mysql_so_SOURCES = sqlite.c
mysql_so_LDADD = $(mysql_libs)
#mysql_so_DEPENDENCES = $(top_builddir)/distdb  
endif
