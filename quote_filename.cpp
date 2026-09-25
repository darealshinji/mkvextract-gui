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
    /**
     * The character is an apostroph which is escaped with a
     * backslash and not surrounded by any quotation marks.
     * Example: \'\'\'
     */
    MODE_APOSTROPH = 0,

    /**
     * Any printable characters including space but excluding apostroph
     * are surrounded by single quotation marks (apostrophs).
     * Example: 'Abc;"* ?\'
     */
    MODE_PRINTABLE = 1,

    /**
     * Any other character is replaced by an escape sequence and
     * surrounded by single quotation marks (apostrophs) that are
     * preceeded by a dollar sign.
     * Example: $'\t\r\n\x01\x02'
     */
    MODE_SHELL_ESC = 2
};


/* convert a byte into a printable escaped hex digit sequence, i.e. \x1B */
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


/* fold text before using it on fl_message() */
void fold_text(std::string &text)
{
    size_t i, cnt;

    for (i=0, cnt=1; i < text.size(); ++i, ++cnt) {
        if (isspace(text[i]) && cnt >= 80) {
            text[i] = '\n';
            cnt = 0;
        }
    }
}


/* quote filenames in a shell compatible way */
std::string quote_filename(const std::string &in)
{
    std::string s;
    char m = MODE_PRINTABLE;

    if (in.empty()) {
        return "''";
    }

    /* set the initial mode from the first character */
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
                /* escaped apostroph */
                s += "\\'";
            } else {
                /* closing single quotation mark and escaped apostroph*/
                s += "'\\'";
            }

            m = MODE_APOSTROPH;
        } else if (isprint(c)) {
            if (m == MODE_APOSTROPH) {
                /* opening single quotation mark */
                s += "'";
            } else if (m == MODE_SHELL_ESC) {
                /* closing and opening single quotation marks */
                s += "''";
            }

            s += c;
            m = MODE_PRINTABLE;
        } else {
            if (m == MODE_PRINTABLE) {
                /* closing single quotation mark plus
                 * begin of escape character sequence */
                s += "'$'";
            } else if (m == MODE_APOSTROPH) {
                /* begin of escape character sequence */
                s += "$'";
            }

            switch (c)
            {
            /* \n\t\r\f\v escape sequences */
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
                /* use a \xNN hex digit sequence for other characters */
                s += uchar_to_hex(c);
                break;
            }

            m = MODE_SHELL_ESC;
        }
    }

    /* append closing single quotation mark if needed */
    if (m != MODE_APOSTROPH) {
        s += "'";
    }

    return s;
}

