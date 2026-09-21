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

#include <string>
#include <ctype.h>


enum {
    MODE_APOSTROPH = 0,
    MODE_PRINTABLE = 1,
    MODE_SHELL_ESC = 2
};


static const char *uchar_to_hex(unsigned char c)
{
    static char buf[8];
    const char *digits = "0123456789ABCDEF";

    buf[0] = '\\';
    buf[1] = 'x';
    buf[2] = digits[c / 16];
    buf[3] = digits[c % 16];
    buf[4] = 0;

    return buf;
}


/* quote filenames in a shell compatible way */
std::string quote_filename(const std::string &in)
{
    std::string s;
    char m = MODE_PRINTABLE;

    if (in.empty()) {
        return "''";
    }

    if (in.front() == '\'') {
        m = MODE_APOSTROPH;
    } else if (isprint(in.front())) {
        m = MODE_PRINTABLE;
        s = "'";
    } else {
        m = MODE_SHELL_ESC;
        s = "$'";
    }

    for (auto &c : in) {
        if (c == '\'') {
            if (m == MODE_APOSTROPH) {
                s += "\\'";
            } else {
                s += "'\\'";
            }

            m = MODE_APOSTROPH;
        } else if (isprint(c)) {
            if (m == MODE_APOSTROPH) {
                s += '\'';
            } else if (m == MODE_SHELL_ESC) {
                s += "''";
            }

            s += c;
            m = MODE_PRINTABLE;
        } else {
            if (m == MODE_PRINTABLE) {
                s += "'$'";
            } else if (m == MODE_APOSTROPH) {
                s += "$'";
            }

            switch (c)
            {
            case '\n':
                s += "\\n";
                break;
            case '\t':
                s += "\\t";
                break;
            case '\r':
                s += "\\r";
                break;
            case '\f':
                s += "\\f";
                break;
            case '\v':
                s += "\\v";
                break;
            default:
                s += uchar_to_hex(c);
                break;
            }

            m = MODE_SHELL_ESC;
        }
    }

    if (m != MODE_APOSTROPH) {
        s += '\'';
    }

    return s;
}

