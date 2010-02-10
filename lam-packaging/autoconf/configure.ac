#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
#	Tim Rice <tim@multitalents.net>
#
# $Id$
#
# Copyright (c) 2005, 2006 Tim Rice
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

AC_PREREQ(2.53)
AC_INIT(ldap-account-manager, @@VERSION@@, lam-public@lists.sourceforge.net)
AC_CONFIG_SRCDIR(README)
AC_PREFIX_DEFAULT(/usr/local/lam)

# work around those that want to build in srcdir
if test x$srcdir = "x." ; then
	exec $0 --srcdir=`pwd` $*
fi

# Does make set variable $(MAKE)
AC_PROG_MAKE_SET

# Checks for programs.
AC_PROG_LN_S
AC_PROG_INSTALL
AC_PATH_PROG(CHMOD, chmod)
AC_PATH_PROG(CHOWN, chown)
AC_PATH_PROG(GREP, grep)
AC_PATH_PROG(ID, id)
AC_PATH_PROG(MKDIR, mkdir)
AC_PATH_PROG(RM, rm)
AC_PATH_PROG(TAR, tar)

# Find a sutable copy command
AC_PATH_PROG(COPY, copy)
AC_PATH_PROG(CP, cp)
if test x$COPY = x ; then
	touch conf-cp-tst
	$CP --preserve=timestamps conf-cp-tst conf-cp-tst2 2>/dev/null
	if test -f conf-cp-tst2 ; then
		COPY_PROG="$CP --preserve=timestamps"
		rm conf-cp-tst2
	fi
	rm conf-cp-tst
else
	COPY_PROG="$COPY -m"
fi
AC_SUBST(COPY_PROG)

HTTPD_USER=httpd
AC_ARG_WITH(httpd-user,
    [  --with-httpd-user=user	Specify the user your http daemon runs as.
				default is http	],
    [
		if test -n "$withval"  &&  test "x$withval" != "xno"  &&  \
		    test "x${withval}" != "xyes"; then
			HTTPD_USER=$withval
		fi
    ]
)
AC_SUBST(HTTPD_USER)

HTTPD_GROUP=httpd
AC_ARG_WITH(httpd-group,
    [  --with-httpd-group=group	Specify the group your http daemon runs as.
				default is http	],
    [
		if test -n "$withval"  &&  test "x$withval" != "xno"  &&  \
		    test "x${withval}" != "xyes"; then
			HTTPD_GROUP=$withval
		fi
    ]
)
AC_SUBST(HTTPD_GROUP)

AC_ARG_WITH(perl,
	[  --with-perl=PATH       Specify path to perl program ],
	[
		if test -n "$withval"  &&  test "x$withval" != "xno"  &&  \
		    test "x${withval}" != "xyes"; then
			perl_path=$withval
		fi
	],
	[
# We search these paths before $PATH to find a more up-to-date/capable
# version of perl than the system installed one.
		TestPath="/opt/bin"
		TestPath="${TestPath}${PATH_SEPARATOR}/opt/sfw/bin"
		TestPath="${TestPath}${PATH_SEPARATOR}/usr/local/bin"
		TestPath="${TestPath}${PATH_SEPARATOR}$PATH"
		AC_PATH_PROGS(perl_path, perl5 perl, , $TestPath)
	]
)

if test -z "$perl_path" ; then
	PERL_PATH="/usr/bin/perl"
	AC_MSG_WARN([*** lamdaemon.pl requires perl ***])
else
	PERL_PATH=$perl_path
fi
AC_SUBST(PERL_PATH)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

