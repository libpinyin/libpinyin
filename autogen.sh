#!/bin/bash

aclocal

autoheader

automake -a

autoconf

./configure --enable-tests
