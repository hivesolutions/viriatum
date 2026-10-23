<?php

// Hive Viriatum Web Server
// Copyright (c) 2008-2026 Hive Solutions Lda.
//
// Minimal script for the mod_php test suite.
// Answers every request the same way, with the method, the address
// and the payload it was reached with written into the body, so
// that the C test harness can tell one request from another.

header("X-Test: viriatum");

echo "method=" . $_SERVER["REQUEST_METHOD"] . "\n";
echo "uri=" . $_SERVER["REQUEST_URI"] . "\n";
echo "input=" . file_get_contents("php://input") . "\n";
