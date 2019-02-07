#!/bin/sh

cd ext
./recipe
cd ..

autoreconf -i ./
