#!/bin/sh
set -e
set -x

test -e fltk || git clone --depth 1 https://github.com/fltk/fltk

FLTK_CONFIG="./fltk/build/usr/bin/fltk-config"

if [ ! -x "$FLTK_CONFIG" ]; then
  rm -rf fltk/build
  mkdir fltk/build
  cd fltk/build

  cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PWD/usr" \
    -DFLTK_OPTION_OPTIM="-Wall -O3 -ffunction-sections -fdata-sections"

  make -j$(nproc)
  make install

  cd ../..
fi

cxxflags="-Wall -O3 -std=c++20 -ffunction-sections -fdata-sections $($FLTK_CONFIG --use-images --cxxflags)"
lflags="-Wl,--gc-sections -Wl,--as-needed -pthread -s"
libs="$($FLTK_CONFIG --use-images --ldflags)"

if pkg-config --exists tinyxml2 ; then
  if [ "x$1" = "x--use-system-tinyxml2" ]; then
    libs="$libs $(pkg-config --libs tinyxml2)"
    cxxflags="$cxxflags -DHAS_TINYXML2"
  fi
fi

g++ $cxxflags mkvextract-gui.cpp -o mkvextract-gui $lflags $libs

