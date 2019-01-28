/*
 Hive Viriatum Commons
 Copyright (c) 2008-2019 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Commons. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2019 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "stdafx.h"

#include "checksum/checksum.h"
#include "compression/compression.h"
#include "crypto/crypto.h"
#include "debug/debug.h"
#include "encoding/encoding.h"
#include "formats/formats.h"
#include "jni/jni.h"
#include "memory/memory.h"
#include "ini/ini.h"
#include "io/io.h"
#include "sorting/sorting.h"
#include "stream/stream.h"
#include "structures/structures.h"
#include "system/system.h"
#include "test/test.h"
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
