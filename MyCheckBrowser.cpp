/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018, djcj <djcj@gmx.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <FL/Fl.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Menu_Item.H>

#include "MyCheckBrowser.hpp"


MyCheckBrowser::MyCheckBrowser(int X, int Y, int W, int H)
 : Fl_Check_Browser(X, Y, W, H, NULL)
{
  box(FL_THIN_DOWN_BOX);
  color(fl_lighter(fl_lighter(FL_BACKGROUND_COLOR)));
  when(FL_WHEN_CHANGED);
}

MyCheckBrowser::~MyCheckBrowser() {
  Fl_Check_Browser::clear();
}

int MyCheckBrowser::handle(int event)
{
  if (event == FL_PUSH) {
    if (Fl::event_button() == FL_RIGHT_MOUSE) {
      const Fl_Menu_Item *m = _menu->popup(Fl::event_x(), Fl::event_y());
      if (m) {
        m->do_callback(NULL);
      }
      return 1;
    }
    deselect();
  }

  return Fl_Check_Browser::handle(event);
}

