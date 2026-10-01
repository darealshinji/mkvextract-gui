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

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <iterator>
#include <string>
#include <vector>

#include "pipe_command.hpp"


pipe_command::pipe_command()
{}


pipe_command::~pipe_command()
{
    pipe_close();
}


FILE *pipe_command::pipe_open(char **argv)
{
    enum { r = 0, w = 1 };
    int fd[2];

    pipe_close();

    if (!argv || pipe2(fd, O_CLOEXEC) == -1) {
        return NULL;
    }

    if ((m_pid = vfork()) != 0) {
        close(fd[w]);
        m_fp = fdopen(fd[r], "r");
        return m_fp;
    }

    close(fd[r]);
    dup2(fd[w], 1);
    close(fd[w]);

    execvp(argv[0], argv);

    _exit(127);
}


FILE *pipe_command::pipe_open(const std::vector<std::string> &argv)
{
    std::vector<char*> vec;

    vec.reserve(argv.size() + 1);

    for (auto &e : argv) {
        vec.push_back(const_cast<char *>(e.c_str()));
    }

    vec.push_back(NULL);

    return pipe_open(std::data(vec));
}


void pipe_command::pipe_close()
{
    if (m_fp) {
        fclose(m_fp);
    }

    if (m_pid > getpid()) {
        kill(m_pid, 1);
    }

    m_fp = NULL;
    m_pid = -1;
}

