#!/usr/bin/env bash

set -e

if [[ -z "$DEVKITPRO" ]]; then
    echo "DEVKITPRO not set!"
    exit 1
fi

"${DEVKITPRO}/portlibs/switch/bin/aarch64-none-elf-cmake" -DSWITCH=1 -DCMAKE_BUILD_TYPE=Release -Bbuild
make -Cbuild -j`nproc`
