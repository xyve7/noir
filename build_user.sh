#!/bin/sh

cd base
tar --format=ustar -cvf ../modules/base.tar *
cd ..
tools/noir-rd/noir-rd -p device/ -f M modules/base.tar
