# Hive Viriatum Web Server
# Copyright (C) 2008-2015 Hive Solutions Lda.
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
# __copyright__ = Copyright (c) 2008-2015 Hive Solutions Lda.
# __license__   = GNU General Public License (GPL), Version 3

echo
echo "System Configuration"
echo "========================"
echo
echo " + Host                  $host_os"
echo " + Install prefix        $prefix"
echo " + Config directory      $with_configroot"
echo " + Data directory        $with_wwwroot"
echo " + Modules directory     $with_moduleroot"
echo " + CC                    $CC"
echo " + CFLAGS                $CFLAGS"
echo " + DEBUG                 $have_debug"
echo " + DEFAULTS              $have_defaults"
echo " + MPOOL                 $have_mpool"
echo " + PREFORK               $have_prefork"
[[ "$CC" == *"clang"* ]] && echo "E CLANG !!!!"
[[ "$CC" != *"clang"* ]] && echo "NAO E CLANG !!!!"
echo " + JNI                   $have_jni"
echo " + Dynamic Linking       $have_dl"
echo " + IPv6 Support          $have_ip6"
echo " + Log                   $have_log"
echo " + Threads               $have_pthread"
echo " + SSL                   $have_ssl"
echo " + PCRE (regex)          $have_pcre"
echo " + WinSockets2           $have_w2_32"
echo " + PSAPI                 $have_psapi"
index=1
for poll_method in $poll_methods; do
    echo " + Polling method $index      $poll_method"
    index=$((index+1))
done
echo
echo Instructions
echo "========================"
echo
echo "Run 'make && make install' to install Viriatum."
echo "Thank you for using Viriatum."
echo
