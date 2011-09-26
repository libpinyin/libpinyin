#!/bin/bash

aclocal

libtoolize --force

autoheader

automake -a

autoconf

./configure --enable-tests
