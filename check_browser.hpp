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

#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Menu_Item.H>


class check_browser : public Fl_Check_Browser
{
private:

    Fl_Menu_Item *m_menu;

    int handle(int event)
    {
        if (m_menu && event == FL_PUSH &&
            Fl::event_button() == FL_RIGHT_MOUSE)
        {
            auto m = m_menu->popup(Fl::event_x(), Fl::event_y());
            if (m) m->do_callback(NULL);
            return 1;
        }

        return Fl_Check_Browser::handle(event);
    }


public:

    check_browser(Fl_Menu_Item *m, int X, int Y, int W, int H)
    : Fl_Check_Browser(X, Y, W, H)
    {
        m_menu = m;
        box(FL_THIN_DOWN_BOX);
        color(fl_lighter(fl_lighter(FL_BACKGROUND_COLOR)));
        when(FL_WHEN_CHANGED);
    }

    virtual ~check_browser() {
        Fl_Check_Browser::clear();
    }

    Fl_Menu_Item *menu() const {
        return m_menu;
    }
};

