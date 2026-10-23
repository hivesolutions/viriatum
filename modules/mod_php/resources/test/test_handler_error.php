<?php

// Hive Viriatum Web Server
// Copyright (c) 2008-2026 Hive Solutions Lda.
//
// Script for the mod_php test suite that fails while it runs.
// Raises an error that is never caught, which is fatal to the
// script and must leave the interpreter able to serve the request
// that comes after it.

throw new RuntimeException("viriatum");
