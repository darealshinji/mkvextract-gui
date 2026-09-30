#!/bin/sh

echo "#pragma once"

if [ -e "fltk/fltk_version.dat" ] && [ -e "fltk/.git" ]; then
    cd fltk
    VERSION=$(cat "fltk_version.dat")
    GIT_HASH=$(git rev-parse --short HEAD)
    echo "#define FLTK_VERSION \"${VERSION} (git snapshot ${GIT_HASH})\""
fi
