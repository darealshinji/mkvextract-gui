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

#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>


struct infos {
    std::string info;
    std::string filename;
};

struct mkv_file_info {
    std::vector<struct infos> tracks;
    std::vector<struct infos> attachments;
    std::vector<int> timestampIDs;
    bool has_chapters;
};


namespace ex {
    int start(const char *in);
}

std::string quote_filename(const std::string &in);

FILE *popen_vp(char **argv, pid_t &child_pid);
FILE *popen_vp(std::vector<std::string> &argv, pid_t &child_pid);

bool parsemkv(std::string &mkv_file, struct mkv_file_info &info, std::string &error);
bool xml2ogm(const char *input, const char *output);

