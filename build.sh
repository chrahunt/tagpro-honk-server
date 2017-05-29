#!/bin/sh
cmake . && make && mkdir build && make install DESTDIR=build
