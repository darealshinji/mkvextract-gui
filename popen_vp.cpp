/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 djcj <djcj@gmx.de>
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

#include <vector>
#include <string>
#include <stdio.h>
#include <unistd.h>


FILE *popen_vp(char **argv, pid_t &child_pid)
{
    enum { r = 0, w = 1 };
    int fd[2];

    if (pipe(fd) == -1) {
        return NULL;
    }

    if ((child_pid = fork()) != 0) {
        close(fd[w]);
        return fdopen(fd[r], "r");
    }

    close(fd[r]);
    dup2(fd[w], 1);
    close(fd[w]);

    execvp(argv[0], argv);

    _exit(127);
}


FILE *popen_vp(std::vector<std::string> &argv, pid_t &child_pid)
{
    size_t len = argv.size();
    char *child_argv[len + 1];

    for (size_t i = 0; i < len; i++) {
        child_argv[i] = const_cast<char *>(argv.at(i).c_str());
    }

    child_argv[len] = NULL;

    return popen_vp(child_argv, child_pid);
}
