# Hive Viriatum Web Server
# Copyright (C) 2008-2012 Hive Solutions Lda.
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
# __revision__  = $LastChangedRevision$
# __date__      = $LastChangedDate$
# __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
# __license__   = GNU General Public License (GPL), Version 3

poll_methods=""
if test "$have_epoll" = true; then poll_methods="${poll_methods}epoll "; fi
if test "$have_kqueue" = true; then poll_methods="${poll_methods}kqueue "; fi
if test "$have_poll" = true; then poll_methods="${poll_methods}poll "; fi
if test "$have_port" = true; then poll_methods="${poll_methods}port "; fi
if test "$have_win32_select" = true; then poll_methods="${poll_methods}win32 "; fi
if test "$have_select" = true; then poll_methods="${poll_methods}select"; fi
