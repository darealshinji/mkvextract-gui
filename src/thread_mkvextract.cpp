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

#include <stdio.h>
#include <string.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/fl_ask.H>
#include <string>
#include <string_view>

#include "check_browser.hpp"
#include "dnd_window.hpp"
#include "pipe_command.hpp"
#include "rotate_box.hpp"
#include "mkvextract.hpp"


void MKVextract::run_mkvextract(std::stop_token &stoken)
{
    std::string base, xml, ogm;

    /* check for command */
    if (!utils::command_in_path("mkvextract")) {
        Fl::lock();

        fl_message_title("Error");
        fl_message("%s", "mkvextract doesn't seem to be in PATH!");

        Fl::unlock();
        Fl::awake();

        return;
    }

    if (stoken.stop_requested()) {
        return;
    }

    Fl::lock();

    m_win->dnd_enabled(false);
    m_use_source_path->deactivate();
    m_but_outdir->deactivate();
    m_but_add->deactivate();

    m_but_extract->deactivate();
    m_but_extract->hide();

    m_but_abort->activate();
    m_but_abort->show();

    m_rotate->start();

    Fl::unlock();
    Fl::awake();

    /* run mkvextract */
    create_cmd(CREATE_VECTOR);
    pipe_command cmd(m_args);

    if (!cmd.open_pipe()) {
        Fl::lock();
        m_progress_box->label("ERROR");
        restore_main_window();
        Fl::unlock();
        Fl::awake();
        return;
    }

    /* read lines */
    constexpr std::string_view gui_progress{"#GUI#progress "};
    constexpr std::string_view gui_error{"#GUI#error "};
    std::string line;

    while (cmd.getline(line)) {
        if (stoken.stop_requested()) {
            return;
        }

        if (line.starts_with(gui_progress)) {
            Fl::lock();
            m_progress_box->copy_label(line.c_str() + gui_progress.size());
            Fl::unlock();
            Fl::awake();
        } else if (line.starts_with(gui_error)) {
            line.erase(0, gui_error.size());
            utils::fold_text(line);

            Fl::lock();

            m_progress_box->label("ERROR");
            restore_main_window();
            fl_message_title("Error");
            fl_message("%s", line.c_str());

            Fl::unlock();
            Fl::awake();

            return;
        }
    }

    Fl::lock();
    m_progress_box->label("DONE");
    restore_main_window();
    Fl::unlock();
    Fl::awake();

    if (stoken.stop_requested()) {
        return;
    }

    /* convert extracted chapters from XML to OGM format */
    if (m_chapters_entry != 0 && m_browser->checked(m_chapters_entry)) {
        base = stem_path();
        xml = base + " - chapters.xml";
        ogm = base + " - chapters.txt";

        if (!utils::xml2ogm(xml, ogm)) {
            Fl::lock();
            fl_message_title("Warning");
            fl_message("Could not create OGM format chapters from file:\n%s!", xml.c_str());
            Fl::unlock();
            Fl::awake();
        }
    }
}

