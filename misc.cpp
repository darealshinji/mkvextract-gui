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

#include <filesystem>
#include <string>
#include <vector>
#include <ctype.h>
#include <stdlib.h>

namespace fs = std::filesystem;



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


static bool is_executable(fs::file_status &st)
{
    auto p = st.permissions();

    if ((p & fs::perms::owner_exec) == fs::perms::none &&
        (p & fs::perms::group_exec) == fs::perms::none &&
        (p & fs::perms::others_exec) == fs::perms::none)
    {
        return false;
    }

    return true;
}


bool command_in_path(const char *command)
{
    const char *env = getenv("PATH");

    if (!env) {
        return false;
    }

    std::vector<std::string> list;
    std::string s;

    auto append_path = [&] () {
        if (!s.ends_with('/')) {
            s += '/';
        }
        s += command;
        list.push_back(s);
    };

    /* split PATH at colon and append command to paths */
    for (const char *p = env; *p != 0; p++) {
        if (*p == ':' && !s.empty()) {
            append_path();
            s.clear();
        } else {
            s += *p;
        }
    }

    if (!s.empty()) {
        append_path();
    }

    /* check if file exists and is executable */
    for (auto &e : list) {
        auto st = fs::status(fs::path(e));

        if (fs::is_regular_file(st) && is_executable(st)) {
            return true;
        }
    }

    return false;
}

