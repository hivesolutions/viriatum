# Hive Viriatum Web Server
# Copyright (C) 2008 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Web Server.
#
# Hive Viriatum Web Server is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Hive Viriatum Web Server is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Hive Viriatum Web Server. If falset, see <http://www.gnu.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision: 2390 $
# __date__      = $LastChangedDate: 2009-04-02 08:36:50 +0100 (qui, 02 Abr 2009) $
# __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
# __license__   = GNU General Public License (GPL), Version 3

AC_ARG_ENABLE(ipv6, AC_HELP_STRING([--disable-ipv6], [Disable IPv6 support]), disabled_ip6=1, disabled_ip6=0)

AC_SEARCH_LIBS(getaddrinfo, socket inet6)
AC_SEARCH_LIBS(getnameinfo, socket inet6)
AC_SEARCH_LIBS(gai_strerror, socket inet6)
AC_CHECK_FUNC(gai_strerror)

APR_CHECK_WORKING_GETADDRINFO
APR_CHECK_WORKING_GETNAMEINFO

AC_ACME_SOCKADDR_UN
AC_ACME_SOCKADDR_IN6
AC_ACME_SOCKADDR_STORAGE

# prints an info message about ipv6
AC_MSG_CHECKING(if the system supports ipv6)
have_ip6=false

if test "$disabled_ip6" = 1; then
    AC_MSG_RESULT([no -- disabled by user])
else
    if test "x$ac_cv_acme_sockaddr_in6" = "xyes"; then
        if test "x$ac_cv_working_getaddrinfo" = "xyes"; then
            if test "x$ac_cv_working_getnameinfo" = "xyes"; then
                have_ip6=true
                AC_MSG_RESULT([yes])
 		        AC_DEFINE(HAVE_IP6, 1, [Define if you have ipv6 support])
            else
                AC_MSG_RESULT([no -- no getnameinfo])
            fi
        else
            AC_MSG_RESULT([no -- no working getaddrinfo])
        fi
    else
        AC_MSG_RESULT([no -- no sockaddr_in6])
    fi
fi