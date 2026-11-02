/**
 Licensed under the MIT License <http://opensource.org/licenses/MIT>.
 SPDX-License-Identifier: MIT
 Copyright (c) 2026 Carsten Janssen

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
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scheme_Choice.H>

#include <string>

#include "check_browser.hpp"
#include "dnd_window.hpp"
#include "mkvextract.hpp"

#if !defined(FLTK_VERSION) && defined(__has_include)
# if __has_include("version.h")
#  include "version.h"
# endif
#endif

#define COL(x) x,x,x


static std::string get_version_string()
{
    std::string s = "Using FLTK version ";

#ifdef FLTK_VERSION
    s += FLTK_VERSION;
# ifdef FLTK_GIT
    s += "\n(git snapshot ";
    s += FLTK_GIT;
# endif
#else
    int version = Fl::api_version();
    int major = version / 10000;
    int minor = (version % 10000) / 100;
    int patch = version % 100;

    s += std::to_string(major);
    s += "." + std::to_string(minor);
    s += "." + std::to_string(patch);
#endif

    return s;
}


void MKVextract::do_colors()
{
    switch (m_choice_colors->value())
    {
    default:
        m_choice_colors->value(THEME_LIGHT);
        [[fallthrough]];

    case THEME_LIGHT:
        Fl::foreground(COL(0)); /* from FL_FOREGROUND_COLOR */
        Fl::background(COL(192)); /* from FL_BACKGROUND_COLOR */
        Fl::background2(COL(255)); /* from FL_BACKGROUND2_COLOR */
        m_browser->color(fl_lighter(FL_BACKGROUND_COLOR));
        break;

    case THEME_DARK:
        Fl::foreground(COL(220));
        Fl::background(COL(50));
        Fl::background2(COL(40));
        m_browser->color(fl_darker(fl_rgb_color(COL(50))));
        break;
    }

    m_win_set->redraw();
    m_win_cmd->redraw();
    m_win_main->redraw();
}


void MKVextract::init_settings_window()
{
    std::stringstream strm;
    int x, y, w, h;
    const int settings_but_w = 170;

    /* settings window */
    w = 350;
    h = 250;
    m_win_set = new Fl_Double_Window(w, h, "Settings");

        /* scheme type */
        x = m_win_set->w() - settings_but_w - 10;
        y = 10;
        w = settings_but_w;
        h = BUT_H;
        auto choice_scheme = new Fl_Scheme_Choice(x, y, w, h, "Scheme");

        /* color type */
        x = choice_scheme->x();
        y = choice_scheme->y() + choice_scheme->h() + 10;
        w = settings_but_w;
        h = BUT_H;
        m_choice_colors = new Fl_Choice(x, y, w, h, "Color theme");
        m_choice_colors->add("light");
        m_choice_colors->add("dark");
        m_choice_colors->value(THEME_LIGHT);
        METHODCB(m_choice_colors, colors);

        /* "center" check button */
        x = m_choice_colors->x();
        y = m_choice_colors->y() + m_choice_colors->h() + 10;
        w = settings_but_w;
        h = BUT_H;
        m_chk_center = new Fl_Check_Button(x, y, w, h, "Try to center windows");

        /* version string */
        h = 60;
        x = 10;
        y = m_win_set->h() - h - 10;
        w = 1;
        auto bx = new Fl_Box(x, y, w, h);
        bx->copy_label(get_version_string().c_str());
        bx->deactivate();
        bx->align(FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM_LEFT);

        /* "Close" button */
        x = m_win_set->w() - BUT_W - 10;
        y = m_win_set->h() - BUT_H - 10;
        w = BUT_W;
        h = BUT_H;
        auto but_close = new Fl_Button(x, y, w, h, "Close");
        static auto close_cb = [](Fl_Widget *o) { o->window()->hide(); };
        but_close->callback(close_cb);

    m_win_set->end();
}

