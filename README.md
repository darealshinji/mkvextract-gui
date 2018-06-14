Simple GUI for mkvextract using FLTK.
Requires mkvinfo and mkvextract to be in PATH.

Easiest way to build is with `fltk-config --compile mkvextract-gui.cpp`

First download FLTK verson 1.3 or 1.4 with either svn or git:
``` sh
svn co --username "" --password "" http://seriss.com/public/fltk/fltk/branches/branch-1.3/ fltk
svn co --username "" --password "" http://seriss.com/public/fltk/fltk/branches/branch-1.4/ fltk
git clone --branch branch-1.3 https://github.com/fltk/test-only fltk
git clone --branch branch-1.4 https://github.com/fltk/test-only fltk
```

Then build FLTK and use fltk-config:
``` sh
mkdir -p fltk/build
cd fltk/build
cmake .. \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX="$PWD/usr" \
	-DOPTION_BUILD_EXAMPLES=OFF \
	-DOPTION_CAIRO=OFF \
	-DOPTION_CAIROEXT=OFF \
	-DOPTION_OPTIM="-Wall -O3" \
	-DOPTION_PRINT_SUPPORT=OFF \
	-DOPTION_USE_GL=OFF \
	-DOPTION_USE_PANGO=OFF \
	-DOPTION_USE_THREADS=ON \
	-DOPTION_USE_XFT=ON
make -j4
make install

cd ../..
./fltk/build/usr/bin/fltk-config --compile mkvextract-gui.cpp
```

You can create a .mkv file for testing:
``` sh
./test/mux.sh
```

