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

#include <stddef.h>
#include <pthread.h>


/* simple wrapper class for pthread */
class posix_thread
{
public:
    typedef void *(*start_routine_t)(void *);

private:
    pthread_t m_thread;
    start_routine_t m_fn;
    void *m_data;
    bool m_init = false;

public:

    posix_thread(start_routine_t fn, void *data = NULL)
    : m_fn(fn), m_data(data)
    {}

    ~posix_thread() {
        stop();
    }

    /* attempt a non-blocking join to check if the thread is still running */
    bool busy() {
        return (m_init && pthread_tryjoin_np(m_thread, NULL) == EBUSY);
    }

    /* don't do anything if a thread is still running */
    void start()
    {
        if (!busy() && m_fn && pthread_create(&m_thread, NULL, m_fn, m_data) == 0) {
            m_init = true;
        }
    }

    /* silently ignores errors if the thread is no longer running */
    void stop()
    {
        if (m_init) {
            pthread_cancel(m_thread);
            pthread_join(m_thread, NULL);
        }
    }
};

