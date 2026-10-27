/**
 Licensed under the MIT License <http://opensource.org/licenses/MIT>.
 SPDX-License-Identifier: MIT
 Copyright (c) 2018-2026 Carsten Janssen

 Permission is hereby  granted, free of charge, to any  person obtaining a copy
 of this software and associated  documentation files (the "Software"), to deal
 in the Software  without restriction, including without  limitation the rights
 to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
 copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
 IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
 FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
 AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
 LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
**/

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Scheme.H>

#include <iostream>
#include <string>

#include "mkvextract.hpp"

#if !defined(FLTK_VERSION) && defined(__has_include)
# if __has_include("version.h")
#  include "version.h"
# endif
#endif


static const char *default_scheme = "gtk+";


static void set_default_icon()
{
    const char *list[] = {
        "/usr/share/icons/hicolor/256x256/apps/mkvextract.png",
        "/usr/share/icons/hicolor/128x128/apps/mkvextract.png",
        "/usr/share/icons/hicolor/64x64/apps/mkvextract.png",
        "/usr/share/icons/hicolor/48x48/apps/mkvextract.png",
        "/usr/share/icons/hicolor/32x32/apps/mkvextract.png",
        "/usr/share/pixmaps/mkvextract.png",
        NULL
    };

    for (auto p = list; *p; p++) {
        Fl_PNG_Image icon(*p);

        if (!icon.fail()) {
            Fl_Double_Window::default_icon(&icon);
            return;
        }
    }
}


static void print_infos()
{
    std::cout << "https://github.com/darealshinji/mkvextract-gui\n";
    std::cout << "using FLTK ";

#ifdef FLTK_VERSION
    std::cout << FLTK_VERSION;
#else
    int version = Fl::api_version();
    int major = version / 10000;
    int minor = (version % 10000) / 100;
    int patch = version % 100;
    std::cout << major << "." << minor << "." << patch;
#endif

    std::cout << " - https://www.fltk.org/" << std::endl;
}


static void print_help(const char *prog)
{
    std::cout << "usage:\n"
        << prog << " -help\n"
        << prog << " [-s scheme] [-dc|-default-colors] file.mkv\n"
        << "schemes:";

    auto names = Fl_Scheme::names();

    for (int i = 0; i < Fl_Scheme::num_schemes(); i++) {
        std::cout << ' ' << names[i];
    }

    std::cout << " (default: " << default_scheme << ")" << std::endl;
}


int main(int argc, char **argv)
{
    /* use GTK+ scheme and system colors by default */
    const char *scheme = default_scheme;
    bool default_colors = false;

    const char *file = NULL;

    for (int i = 1; i < argc; i++) {
        std::string arg{argv[i]};

        if (arg == "-help" || arg == "--help") {
            print_help(argv[0]);
            return 0;
        } else if (arg == "-dc" || arg == "-default-colors") {
            default_colors = true;
        } else if (arg == "-s" && (i+1) < argc) {
            scheme = argv[++i];
        } else {
            file = argv[i];
        }
    }

    print_infos();
    set_default_icon();

    Fl::scheme(scheme);

    if (!default_colors) {
        Fl::get_system_colors();
    }

    MKVextract ex;
    ex.show(file);

    return Fl::run();
}
