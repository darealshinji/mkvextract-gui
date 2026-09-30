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
#include <string>
#include <vector>
#include <stdio.h>

#include "rotate_box.hpp"



static const char *svg_template =
  "<svg width=\"1024\" height=\"1024\" version=\"1.1\" viewBox=\"0 0 270.93 270.93\">"
    "<g transform=\"matrix(.82922 0 0 .82922 23.133 1.5165)\" stroke-width=\"0\">"
      "<rect transform=\"rotate(0)\"   x=\"0\"       y=\"152.81\"  width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#@@@\" />"
      "<rect transform=\"rotate(45)\"  x=\"74.544\"  y=\"9.7088\"  width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#@@@\" />"
      "<rect transform=\"rotate(90)\"  x=\"26.067\"  y=\"-144.19\" width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#@@@\" />"
      "<rect transform=\"rotate(135)\" x=\"-117.03\" y=\"-218.73\" width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#@@@\" />"
      "<rect transform=\"rotate(0)\"   x=\"181.1\"   y=\"152.81\"  width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#@@@\" />"
      "<rect transform=\"rotate(45)\"  x=\"255.64\"  y=\"9.7088\"  width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#@@@\" />"
      "<rect transform=\"rotate(90)\"  x=\"207.17\"  y=\"-144.19\" width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#@@@\" />"
      "<rect transform=\"rotate(135)\" x=\"64.064\"  y=\"-218.73\" width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#@@@\" />"
    "</g>"
  "</svg>";


template<size_t N>
bool find_at_pos(const std::string &haystack, char const (&needle)[N], size_t &pos) {
    return ((pos = haystack.find(needle, pos, N-1)) != std::string::npos);
}


void rotate_box::next_frame_cb(void *p) {
    reinterpret_cast<rotate_box *>(p)->do_next_frame();
}


void rotate_box::do_next_frame()
{
    if (++m_frame == m_array.end()) {
        m_frame = m_array.begin();
    }

    image(*m_frame);
    parent()->redraw();

    Fl::repeat_timeout(m_time, next_frame_cb, this);
}


rotate_box::rotate_box(int X, int Y, int W, int H)
: Fl_Box(FL_NO_BOX, X, Y, W, H, NULL)
{
    std::string copy;

    /* number of images to generate */
    const int size = 8;

    /* color values */
    const char *col = "59d00000" "59d00000";
    const char *p = col+size;  /* ^ here */

    m_array.reserve(size);

    /* save icons in array; decreasing the pointer "p"
     * makes the color values shift/rotate to the right */
    for (int i = 0; i < size; ++i, --p) {
        size_t pos = 0;
        copy = svg_template;

        /* insert color values */
        for (int j = 0; j < size; ++j) {
            if (find_at_pos(copy, "@@@", pos)) {
                copy.replace(pos, 3, 3, p[j]);
                //putchar(p[j]);
            }
        }

        m_array.push_back(new Fl_SVG_Image(NULL, copy.c_str()));
        m_array.back()->resize(W, H);
        //putchar('\n');
    }
}


rotate_box::~rotate_box()
{
    stop();

    while (!m_array.empty()) {
        delete m_array.back();
        m_array.pop_back();
    }
}


void rotate_box::start()
{
    if (m_running) {
        return;
    }

    m_frame = m_array.begin();
    image(*m_frame);

    Fl::add_timeout(m_time, next_frame_cb, this);
    m_running = true;
}


void rotate_box::stop()
{
    if (m_running) {
        Fl::remove_timeout(next_frame_cb);
    }

    image(NULL);
    m_running = false;
}

