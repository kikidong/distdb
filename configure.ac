
AC_PREREQ([2.63])

AC_INIT([distdb], [0.0.1], [microcai@fedoraproject.org])

AC_CONFIG_HEADERS([config.h])
AC_DEFINE(AUTHOR,"microcai",[Author of this software])

AC_CONFIG_SRCDIR([init/main.c])

AM_INIT_AUTOMAKE([-Wall -Werror])
# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

#AC_PROG_LIBTOOL
#AC_LIBTOOL_WIN32_DLL
#AC_DISABLE_STATIC

#PKG_PROG_PKG_CONFIG
AC_CHECK_PROG([have_curl],[curl],[yes],[no])
AS_IF([ test x"$have_curl" == xno],[AC_MSG_ERROR([curl not found!])])

#AC_PROG_RANLIB

#Check for WITH
AC_ARG_WITH(config,
	[AC_HELP_STRING([--with-config=file],[use confile file instead of default(/etc/distdb.conf)])],
	[],
	[with_config="/etc/distdb.conf"]
)

AC_ARG_WITH(nodes_url,
	[AC_HELP_STRING([--with-nodes_url=URL],[use URL for nodes download location. can be changed by editting conf file])],
	[],
	[with_nodes_url="http://`hostname`.`domainname`/nodes.db"]
)

AC_ARG_WITH([sqlite],
			[AC_HELP_STRING([--with-sqlite],[build support for sqlite kackend, default yes])],[],
			[with_sqlite="check"])
			
AC_ARG_WITH([oci],
			[AC_HELP_STRING([--with-oci],[build support for oracel kackend, default no])],[],
			[with_oci="check"])

AC_ARG_WITH([mysql],
			[AC_HELP_STRING([--with-mysql],[build support for mysql kackend, default no])],[],
			[with_mysql="no"])

#Check for ENABLE
AC_ARG_ENABLE([debug],[AC_HELP_STRING([--enable-debug],[enable debug])],
		[
			CFLAGS="-O0 -g3"
			AC_DEFINE(DEBUG,1,[defined if in debug mode])
		])

# Checks for libraries.
#PKG_CHECK_MODULES([LIBCURL],[libcurl])
#AC_CHECK_LIB([curl],[curl_easy_init])
AC_CHECK_LIB(pthread,pthread_create,[],[AC_MSG_FAILURE([need pthread library],1)])
AC_SEARCH_LIBS(dlopen,dl)

AS_IF([ test "x$with_sqlite" != "xno" ],
	[
		AC_CHECK_LIB(sqlite3,sqlite3_open,
			[
				AC_CHECK_HEADER([sqlite3.h],[have_sqlite="yes"] , 
				[AC_MSG_WARN(sqlite3 not found on your system!)])
			])
])

AM_CONDITIONAL([WITH_SQLITE], [ test "x$have_sqlite" = "xyes" ] )

AS_IF([ test "x$with_oci" != "xno" ],
	[
		AC_CHECK_LIB(oci,oci_open,
			[
				AC_CHECK_HEADER([oci.h],[have_oci="yes"] , 
				[AC_MSG_WARN(oracel client not found on your system!)])
			])
])

AM_CONDITIONAL([WITH_OCI], [ test "x$have_oci" = "xyes" ] )
		
AS_IF([ test "x$with_mysql" != "xno" ],
	[
	AC_CHECK_PROG([have_mysql_config],[mysql_config],[yes],[no])
	AS_IF([test "x$have_mysql_config" = "xyes" ],
		[
			AC_CHECK_HEADER([mysql/mysql.h],
				[
					AC_SUBST(mysql_libs,[`mysql_config --libs`])
					have_mysql=yes
				],[
				AC_MSG_WARN([mysql client found on your system, but mysql/mysql.h not found! Is that OK?])
				])	
		],
		[AC_MSG_WARN(mysql client not found on your system!)]
	)
])

AM_CONDITIONAL([WITH_MYSQL], [ test "x$have_mysql" = "xyes" ] )

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE


AC_CHECK_DECL([INET_ADDRSTRLEN],[],
	[AC_DEFINE([INET_ADDRSTRLEN],[16],[sizeof(sockaddr_in)])],
	[[#include <netinet/in.h>
	]])

# Checks for library functions.
AC_CHECK_FUNC([popen],[],[AC_MSG_ERROR([popen not defined!])])
AC_CHECK_FUNC([getline],[AC_DEFINE(HAVE_GETLINE,1,[more safe than fgets, defined if found ])],
				[AC_MSG_WARN([getline not found, use unsafe fgets instead!])])
AC_CHECK_FUNC([getdelim],[AC_DEFINE(HAVE_GETDELIM,1,[more safe than fgets, defined if found ])],
				[AC_MSG_WARN([getdelim not found, use unsafe fgets instead!])])

AC_CHECK_TYPE(uint32_t,[],
	[
		AC_DEFINE(uint32_t,UINT,[Define for windows that do not have uint32_t])
	],
	[[
		#include <stdint.h>
	]])
AC_CHECK_HEADERS([stdint.h])
#define MACROS

AC_DEFINE_UNQUOTED(CONF_FILE,"${with_config}",[default config file])

AS_IF( [ test "${localstatedir}" = '$'"{prefix}/var"],[localstatedir=/var/distdb])

AS_IF( [ test "${sysconfdir}" = '$'"{prefix}/etc"],[sysconfdir=/etc])

AS_IF( [ test "${libdir}" = '$'"{exec_prefix}/lib" ], [
	AS_IF( [test "${prefix}" = "NONE"], [libdir=/usr/local/lib] , [libdir=${prefix}/lib]   )
	] )

AC_SUBST(NODES_URL,[${with_nodes_url}])
LIB_VERSION_MAJOR=0
LIB_VERSION_MINOR=0
LIB_VERSION_CURRENT=1
AC_SUBST(LIB_VERSION,"${LIB_VERSION_MAJOR}:${LIB_VERSION_CURRENT}:${LIB_VERSION_MINOR}")
AC_DEFINE(RPC_DEFAULT_PORT,3721,[default port])
AC_DEFINE(DISTDB_DEFAULT_PORT,3722,[default port])

AC_DEFINE_UNQUOTED(PLUINGDIR,"${libdir}/distdb/",[The dir that hold plugin])

#OUTPUT files
AC_CONFIG_FILES([				 
				 nodes.db				 
                 include/Makefile
                 src/Makefile
                 lib/Makefile
                 init/Makefile
                 db/Makefile
                 doc/Makefile
                 doc/Doxyfile
                 Makefile
                 distdb.conf                 
                 ])
AC_OUTPUT
