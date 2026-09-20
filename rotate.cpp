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
#include <FL/Fl_Group.H>
#include <FL/Fl_SVG_Image.H>
#include <vector>
#include <stdio.h>

#include "rotate.hpp"


#define TIMEOUT_SPEED 0.1 /* seconds */


static const char svg_template[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<svg width=\"1024\" height=\"1024\" version=\"1.1\" viewBox=\"0 0 270.93 270.93\""
    " xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">"
        "<g transform=\"matrix(.82922 0 0 .82922 23.133 1.5165)\" stroke-width=\"0\">"
            "<rect transform=\"rotate(0)\"   x=\"0\"       y=\"152.81\"  width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
            "<rect transform=\"rotate(45)\"  x=\"74.544\"  y=\"9.7088\"  width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
            "<rect transform=\"rotate(90)\"  x=\"26.067\"  y=\"-144.19\" width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
            "<rect transform=\"rotate(135)\" x=\"-117.03\" y=\"-218.73\" width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
            "<rect transform=\"rotate(0)\"   x=\"181.1\"   y=\"152.81\"  width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
            "<rect transform=\"rotate(45)\"  x=\"255.64\"  y=\"9.7088\"  width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
            "<rect transform=\"rotate(90)\"  x=\"207.17\"  y=\"-144.19\" width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
            "<rect transform=\"rotate(135)\" x=\"64.064\"  y=\"-218.73\" width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
        "</g>"
    "</svg>";


void rotate::next_frame()
{
    if (m_box) {
        if (++m_frame == m_array.end()) {
            m_frame = m_array.begin();
        }

        m_box->image(*m_frame);
        m_box->parent()->redraw();

        Fl::repeat_timeout(TIMEOUT_SPEED, cb_next, this);
    }
}


void rotate::cb_next(void *p) {
    reinterpret_cast<rotate *>(p)->next_frame();
}


rotate::rotate(Fl_Box *box, int size) : m_box(box)
{
    /* color values */
    std::vector<const char *> v = {
        "555", "999", "ddd", "000",
        "000", "000", "000", "000"
    };

    std::vector<char> buf(sizeof(svg_template) + v.size()*3);

    for (size_t i = 0; i < v.size(); i++) {
        if (i > 0) {
            /* rotate/shift color entries */
            v.insert(v.begin(), v.back());
            v.pop_back();
        }

        snprintf(std::data(buf), buf.size(), svg_template,
            v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
        m_array.push_back(new Fl_SVG_Image(NULL, std::data(buf)));
        m_array.back()->resize(size, size);
    }
}


rotate::~rotate()
{
    deactivate();

    while (!m_array.empty()) {
        delete m_array.back();
        m_array.pop_back();
    }
}


void rotate::activate()
{
    if (m_box) {
        m_frame = m_array.begin();
        m_box->image(*m_frame);
        Fl::add_timeout(TIMEOUT_SPEED, cb_next, this);
    }
}


void rotate::deactivate()
{
    if (m_box) {
        Fl::remove_timeout(cb_next);
        m_box->image(NULL);
    }
}

