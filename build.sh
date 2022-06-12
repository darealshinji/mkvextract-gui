#!/bin/sh
set -e
set -x

test -e fltk || git clone --depth 1 https://github.com/fltk/fltk

mkdir -p fltk/build
cd fltk/build

cmake .. \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX="$PWD/usr" \
	-DOPTION_OPTIM="-Wall -O3 -ffunction-sections -fdata-sections" \
	-DOPTION_USE_THREADS=ON \
	-DOPTION_USE_XFT=ON

make -j$(nproc)
make install

cd ../..

cflags="-Wall -O3 -ffunction-sections -fdata-sections $(./fltk/build/usr/bin/fltk-config --use-images --cflags)"
lflags="-Wl,--gc-sections -Wl,--as-needed -pthread"
libs="$(./fltk/build/usr/bin/fltk-config --use-images --ldflags) -lX11 -lpthread -lm -ldl"

pkg-config --libs tinyxml2 2>/dev/null >/dev/null
if [ $? = 0 ] && [ "x$1" = "x--use-system-tinyxml2" ]; then
  libs="$libs $(pkg-config --libs tinyxml2)"
  cflags="$cflags -DHAS_TINYXML2"
fi

g++ $cflags mkvextract-gui.cpp -o mkvextract-gui $lflags $libs
strip mkvextract-gui

