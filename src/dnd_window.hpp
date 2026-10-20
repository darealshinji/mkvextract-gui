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

#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>


class dnd_window : public Fl_Double_Window
{
private:

    /* A dnd box will be shown when a dnd event has started
     * and it will be hidden when it has stopped.
     * That way the overlaying dnd box won't interfere with
     * other widgets when it's not needed. */
    class dnd_box : public Fl_Box
    {
    private:

        int handle(int event)
        {
            int rv = Fl_Box::handle(event);

            switch (event)
            {
            case FL_DND_ENTER:
            case FL_DND_DRAG:
            case FL_DND_LEAVE:
            case FL_DND_RELEASE:
                rv = 1;
                break;
            case FL_PASTE:
                /* hide box on paste */
                hide();
                do_callback();
                rv = 1;
                break;
            }

            return rv;
        }

    public:

        dnd_box(int X, int Y, int W, int H)
        : Fl_Box(FL_NO_BOX, X, Y, W, H, NULL)
        {}

        virtual ~dnd_box()
        {}
    };


    dnd_box *m_dnd = NULL;
    Fl_Callback *m_dnd_callback = NULL;
    void *m_dnd_user_data = NULL;
    bool m_dnd_enabled = false;

    int handle(int event)
    {
        if (dnd_enabled()) {
            if (event == FL_DND_ENTER) {
                m_dnd->show();
            } else if (event == FL_DND_LEAVE) {
                m_dnd->hide();
            }
        }

        return Fl_Double_Window::handle(event);
    }

public:

    dnd_window(int W, int H, const char *L = NULL)
    : Fl_Double_Window(W, H, L)
    {}

    virtual ~dnd_window()
    {}

    void dnd(Fl_Callback *dnd_callback, void *user_data) {
        m_dnd_callback = dnd_callback;
        m_dnd_user_data = user_data;
    }

    bool dnd_enabled() const { return (m_dnd_enabled && m_dnd && m_dnd_callback); }
    void dnd_enabled(bool b) { m_dnd_enabled = b; }

    void end()
    {
        if (!m_dnd) {
            m_dnd = new dnd_box(0, 0, w(), h());
            m_dnd->hide();
        }

        if (m_dnd_callback) {
            m_dnd->callback(m_dnd_callback, m_dnd_user_data);
        }

        Fl_Double_Window::end();
    }
};

