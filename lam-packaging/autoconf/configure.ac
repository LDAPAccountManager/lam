#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
#	Tim Rice <tim@multitalents.net>
#
# $Id$
#
# Copyright (c) 2005, 2006, 2010 Tim Rice
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
#
#The complete license can be found in the file COPYING.

AC_PREREQ(2.53)
AC_INIT(ldap-account-manager, @@VERSION@@, lam-public@lists.sourceforge.net)
AC_CONFIG_SRCDIR(README)
AC_PREFIX_DEFAULT(/usr/local/lam)

# work around those that want to build in srcdir
if test x$srcdir = "x." ; then
	srcdir=`pwd`
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

if test ${prefix} = NONE; then
	WEB_ROOT=${ac_default_prefix}
else
	WEB_ROOT=${prefix}
fi
AC_ARG_WITH(web-root,
    [  --with-web-root=PATH	Specify where the LAM files that the web server
			uses should be installed.
			default is ${prefix}	],
    [
		if test -n "$withval"  &&  test "x$withval" != "xno"  &&  \
		    test "x${withval}" != "xyes"; then
			WEB_ROOT=$withval
		fi
    ]
)
AC_SUBST(WEB_ROOT)

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

