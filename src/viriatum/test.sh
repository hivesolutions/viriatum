#!/bin/bash
# -*- coding: utf-8 -*-

DIR=$(dirname $(readlink -f ${BASH_SOURCE[0]}))

$DIR/viriatum --test
exit $?
