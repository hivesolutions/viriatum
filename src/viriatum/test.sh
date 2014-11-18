#!/bin/bash
# -*- coding: utf-8 -*-

DIR=$(dirname $(readlink -f ${BASH_SOURCE[0]}))

echo "Starting ..."

$DIR/viriatum --help
exit $?
