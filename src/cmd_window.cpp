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
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <string>

#include "mkvextract.hpp"


void MKVextract::do_copy_command()
{
    /* get a local copy first */
    std::string str = m_txtbuf->text_str();

    /* copy text to clipboard */
    Fl::copy(str.c_str(), str.length(), COPY_CLIPBOARD);
}


void MKVextract::init_cmd_window()
{
    int x, y, w, h;

    /* text buffer */
    m_txtbuf = new Fl_Text_Buffer();

    /* command line window */
    w = CMD_W;
    h = CMD_H;
    m_win_cmd = new Fl_Double_Window(w, h, "Command line");

        /* text display */
        x = 10;
        y = 10;
        w = m_win_cmd->w() - 20;
        h = m_win_cmd->h() - 20 - BUT_H;
        auto txt = new Fl_Text_Display(x, y, w, h);
        txt->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 2);
        txt->buffer(m_txtbuf);

        /* button group */
        x = 0;
        y = txt->y() + txt->h();
        w = m_win_cmd->w();
        h = m_win_cmd->h() - y;
        auto g_bttn = new Fl_Group(x, y, w, h);

            /* "Copy" button */
            x = g_bttn->w() - 10 - BUT_W;
            y = g_bttn->y() + 5;
            w = BUT_W;
            h = BUT_H;
            auto btcopy = new Fl_Button(x, y, w, h, "Copy");
            METHODCB(btcopy, copy_command);

            /* dummy */
            x = btcopy->x() - 1;
            y = btcopy->y();
            w = 1;
            h = 1;
            auto dummy = new Fl_Box(FL_NO_BOX, x, y, w, h, NULL);

        g_bttn->end();
        g_bttn->resizable(dummy);

    m_win_cmd->end();
    m_win_cmd->resizable(txt);
    m_win_cmd->size_range(CMD_MIN_W, CMD_MIN_H, Fl::w(), Fl::h());
}

