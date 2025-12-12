#!/bin/bash

target="$1"

if [ -z "$target" ]; then
    target="Makefile"
else
    if [ "$target" = "All" ]; then
        target="Makefile"
    else
        target="$target.make"
    fi
fi

cd build
make -f "$target"

cd -

sudo ./.scripts/update.sh