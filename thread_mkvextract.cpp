/**
 Licensed under the MIT License <http://opensource.org/licenses/MIT>.
 SPDX-License-Identifier: MIT
 Copyright (c) 2018-2026 Carsten Janssen

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

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "pipe_command.hpp"
#include "mkvextract.hpp"


/* this function is called by pthread_create() */
void *MKVextract::thread_run_mkvextract(void *p)
{
    reinterpret_cast<MKVextract *>(p)->run_mkvextract();
    return NULL;
}


void MKVextract::run_mkvextract()
{
    std::string base, xml, ogm;

    if (!command_in_path("mkvextract")) {
        Fl::lock();

        fl_message_title("Error");
        fl_message("%s", "mkvextract doesn't seem to be in PATH!");

        Fl::unlock();
        Fl::awake();

        return;
    }

    create_cmd(true);

    Fl::lock();

    m_dnd_area->deactivate();
    m_use_source_path->deactivate();
    m_but_outdir->deactivate();
    m_but_add->deactivate();

    m_but_extract->label("Abort");
    m_but_extract->callback(abort_cb, this);
    m_rotate->activate();

    Fl::unlock();
    Fl::awake();

    pipe_command cmd(m_args);
    FILE *fp = cmd.pipe_open();

    if (!fp) {
        Fl::lock();
        m_progress_box->label("ERROR");
        restore_main_window();
        Fl::unlock();
        Fl::awake();
        return;
    }

    size_t n = 0;
    char *buf = NULL;
    auto_free af(buf);

    while (getline(&buf, &n, fp) != -1) {
        if (strncmp(buf, "#GUI#progress ", 14) == 0) {
            Fl::lock();
            m_progress_box->copy_label(buf + 14); /* trailing newline is ignored */
            Fl::unlock();
            Fl::awake();
        } else if (strncmp(buf, "#GUI#error ", 11) == 0) {
            std::string msg = buf + 11;
            fold_text(msg);

            Fl::lock();

            m_progress_box->label("ERROR");
            restore_main_window();
            fl_message_title("Error");
            fl_message("%s", msg.c_str());

            Fl::unlock();
            Fl::awake();

            return;
        }
    }

    cmd.pipe_close();

    Fl::lock();
    m_progress_box->label("DONE");
    restore_main_window();
    Fl::unlock();
    Fl::awake();

    /* convert extracted chapters from XML to OGM format */
    if (m_chapters_entry != 0 && m_browser->checked(m_chapters_entry)) {
        if (m_use_source_path->value() == true) {
            base = m_outdir_source + file_stem(m_file);
        } else {
            base = m_outdir_manual + file_stem(m_file);
        }

        xml = base + " - chapters.xml";
        ogm = base + " - chapters.txt";

        if (!xml2ogm(xml.c_str(), ogm.c_str())) {
            Fl::lock();
            fl_message_title("Warning");
            fl_message("Could not create OGM format chapters from file:\n%s!", xml.c_str());
            Fl::unlock();
            Fl::awake();
        }
    }
}

