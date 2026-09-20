#!/bin/sh
set -e
set -x

#if [ ! -e fltk ]; then
#    git clone https://github.com/fltk/fltk
#fi

rm -rf fltk/build
mkdir -p fltk/build
cd fltk/build

# apt install --no-install-recommends libjpeg-dev libpng-dev libfreetype-dev libpango1.0-dev libx11-dev libxft-dev libxext-dev libxinerama-dev libxrender-dev

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PWD/usr" \
    -DCMAKE_CXX_STANDARD=20 \
    -DFLTK_BACKEND_WAYLAND=OFF \
    -DFLTK_BACKEND_X11=ON \
    -DFLTK_BUILD_EXAMPLES=OFF \
    -DFLTK_BUILD_FLTK_OPTIONS=OFF \
    -DFLTK_BUILD_FLUID=OFF \
    -DFLTK_BUILD_FORMS=OFF \
    -DFLTK_BUILD_GL=OFF \
    -DFLTK_BUILD_SCREENSHOTS=OFF \
    -DFLTK_BUILD_TEST=OFF \
    -DFLTK_GRAPHICS_CAIRO=OFF \
    -DFLTK_OPTION_OPTIM="-ffunction-sections -fdata-sections" \
    -DFLTK_OPTION_CAIRO_EXT=OFF \
    -DFLTK_OPTION_CAIRO_WINDOW=OFF \
    -DFLTK_OPTION_FILESYSTEM_SUPPORT=ON \
    -DFLTK_OPTION_LARGE_FILE=ON \
    -DFLTK_OPTION_PEN_SUPPORT=ON \
    -DFLTK_OPTION_PRINT_SUPPORT=OFF \
    -DFLTK_OPTION_SVG=ON \
    -DFLTK_USE_PANGO=ON \
    -DFLTK_USE_PTHREADS=ON \
    -DFLTK_USE_SYSTEM_LIBJPEG=ON \
    -DFLTK_USE_SYSTEM_LIBPNG=ON \
    -DFLTK_USE_SYSTEM_ZLIB=ON \
    -DFLTK_USE_XFT=ON \
    -DFLTK_USE_XINERAMA=ON \
    -DFLTK_USE_XRENDER=ON

#make -j$(nproc)
make -j4
make install

