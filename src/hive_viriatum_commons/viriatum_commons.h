/*
 Hive Viriatum Commons
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Commons. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#include "stdafx.h"

#include "checksum/checksum.h"
#include "compression/compression.h"
#include "debug/debug.h"
#include "encoding/encoding.h"
#include "jni/jni.h"
#include "memory/memory.h"
#include "ini/ini.h"
#include "io/io.h"
#include "sorting/sorting.h"
#include "stream/stream.h"
#include "structures/structures.h"
#include "system/system.h"
#include "template/template.h"
#include "thread/thread.h"
#include "util/util.h"

#ifdef VIRIATUM_PLATFORM_MSC
#ifdef VIRIATUM_DEBUG
#pragma comment(lib, "viriatum_commons_d.lib")
#else
#pragma comment(lib, "viriatum_commons.lib")
#endif
#endif
