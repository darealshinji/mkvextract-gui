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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>

#include <sstream>
#include <string>
#include <utility>

#include "check_browser.hpp"
#include "dnd_window.hpp"
#include "fnfc.hpp"
#include "posix_thread.hpp"
#include "rotate_box.hpp"
#include "mkvextract.hpp"


/* inline lambda start routine for pthread_create() */
#define PTHREADCB(METHOD) \
    [] (void *p) -> void* { \
        reinterpret_cast<decltype(this)>(p)->METHOD(); \
        return NULL; \
    }

/* generate callback function (lambda) */
#define MAKE_CALLBACK(NAME) \
    static auto NAME##_cb = [] (Fl_Widget *, void *p) { \
        reinterpret_cast<decltype(this)>(p)->do_##NAME(); \
    }

/* generate callback function and set callback */
#define METHODCB(WIDGET, NAME) \
    do { \
        MAKE_CALLBACK(NAME); \
        WIDGET->callback(NAME##_cb, this); \
    } while (0)


enum {
    /* buttons */
    BUT_H          = 28,
    BUT_W          = 110,

    /* main window */
    DEFAULT_W      = 800,
    DEFAULT_H      = 480,
    MINIMUM_W      = 400,
    MINIMUM_H      = 200,

    /* command line window */
    CMD_W          = 640,
    CMD_H          = 320,
    CMD_MIN_W      = 160,
    CMD_MIN_H      = 100,

    /* Fl::copy() options */
    COPY_SELECTION = 0,
    COPY_CLIPBOARD = 1,
    COPY_BOTH      = 2
};


static const char *default_title = "simple mkvextract GUI";


static inline std::string move_str(char *ptr)
{
    std::string s = ptr;
    free(ptr);

    return s;
}


static inline std::string absolute_str(const char *file)
{
    if (!file || !*file) {
        return "";
    }

    char *ptr = realpath(file, NULL);

    return ptr ? move_str(ptr) : file;
}


static inline std::string get_cwd_str()
{
    char *ptr = get_current_dir_name();

    return ptr ? move_str(ptr) : absolute_str(".");
}


void MKVextract::restore_main_window()
{
    m_but_outdir->activate();
    m_but_add->activate();

    m_but_abort->deactivate();
    m_but_abort->hide();

    m_but_extract->activate();
    m_but_extract->show();

    m_win->dnd_enabled(true);
    m_use_source_path->activate();
    m_rotate->stop();

    m_win->redraw();
}


void MKVextract::do_dnd()
{
    const char *text = Fl::event_text();

    if (!text || !*text) {
        return;
    }

    /* find newline separator */
    const char *p = strchr(text, '\n');

    if (!p) {
        return;
    }

    /* copy first item, without newline */
    std::string str(text, p - text);

    if (str.starts_with("file://")) {
        /* decode URI in place */
        str.erase(0, 7);
        fl_decode_uri(std::data(str));
    }

    if (!str.starts_with('/')) {
        /* not a full path */
        return;
    }

    stop_threads();

    /* get file infos */
    m_file = std::move(str);
    m_th_info->start();
}


void MKVextract::do_set_outdir()
{
    FNFC fc(FNFC::BROWSE_DIRECTORY, "Set output directory");
    fc.options(FNFC::NEW_FOLDER);

    const char *dir = fc.select();

    if (!dir || !*dir) {
        return;
    }

    m_outdir_manual = dir;

    if (!m_outdir_manual.ends_with('/')) {
        m_outdir_manual += '/';
    }

    m_outdir_field->label(m_outdir_manual.c_str());
    m_outdir_field->activate();
    m_use_source_path->clear();
}


void MKVextract::do_open_file()
{
    FNFC fc(FNFC::BROWSE_FILE, "Select a file");
    fc.filter("Matroska\t*.mkv,*.mka,*.mks,*.mk3d,*.webm");

    const char *file = fc.select();

    if (file && *file) {
        stop_threads();
        m_file = file;
        m_th_info->start();
    }
}


void MKVextract::do_copy_command()
{
    /* get a local copy first */
    std::string str = m_txtbuf->text_str();

    /* copy text to clipboard */
    Fl::copy(str.c_str(), str.length(), COPY_CLIPBOARD);
}


void MKVextract::do_cmd()
{
    create_cmd(CREATE_TEXTBUFFER);

    /* resize window to default and
     * position at center above main window */
    int x = m_win->x() + ((m_win->w() - CMD_W) / 2);
    int y = m_win->y() + ((m_win->h() - CMD_H) / 2);
    m_cmd->resize(x, y, CMD_W, CMD_H);
    m_cmd->show();
}


void MKVextract::do_extract()
{
    stop_threads();
    m_cmd->hide();
    m_win->redraw();

    /* start extraction */
    m_th_extract->start();
}


void MKVextract::do_abort()
{
    stop_threads();
    m_progress_box->label("STOPPED");
    restore_main_window();
}


void MKVextract::do_check_outdir()
{
    if (m_use_source_path->value() == true) {
        /* same as input file directory */
        m_outdir_field->label(m_outdir_source.c_str());
        m_outdir_field->deactivate();
    } else {
        /* manually selected output directory */
        m_outdir_field->label(m_outdir_manual.c_str());
        m_outdir_field->activate();
    }
}


void MKVextract::do_update_browser()
{
    /* "Select ..." menu entries */
    auto sel_all = m_browser->menu();
    auto sel_none = sel_all->next();
    auto sel_attach = sel_none->next();

    if (m_attach_count > 0) {
        sel_attach->activate();
    }

    /* nothing selected */
    if (m_browser->nchecked() == 0) {
        m_but_extract->deactivate();
        m_but_cmd->deactivate();
        m_progress_box->label(NULL);

        sel_none->deactivate();
        sel_all->activate();
        return;
    }

    /* one or more items selected */

    m_but_extract->activate();
    m_but_cmd->activate();
    m_progress_box->label("READY");

    sel_none->activate();

    if (m_browser->checked_all()) {
        sel_all->deactivate();
        sel_attach->deactivate();
    } else {
        sel_all->activate();

        /* all attachments selected? */
        size_t start = m_track_count + 1;
        size_t end = m_track_count + m_attach_count;

        if (m_browser->range_checked(start, end)) {
            sel_attach->deactivate();
        }
    }
}


void MKVextract::do_select_all()
{
    m_browser->check_all();
    do_update_browser();
}


void MKVextract::do_select_none()
{
    m_browser->check_none();
    do_update_browser();
}


void MKVextract::do_select_attach()
{
    int line = m_track_count + 1;

    for (size_t i = 0; i < m_attach_count; i++, line++) {
        m_browser->set_checked(line);
    }

    do_update_browser();
}


void MKVextract::do_quit()
{
    /* ignore Escape key on main window */
    if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape) {
        return;
    }

    /* warn if extraction is still in progress */
    if (m_th_extract->busy()) {
        fl_message_title(default_title);
        int rv = fl_choice("%s", "Cancel", "Close", "Don't close",
                           "Extraction is still in progress.\n"
                           "Close the program anyway?");
        if (rv != 1) {
            return;
        }
    }

    stop_threads();
    Fl::hide_all_windows();
}


void MKVextract::init_main_window()
{
    const int center_align = FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP;
    const int left_align   = FL_ALIGN_LEFT   | FL_ALIGN_INSIDE | FL_ALIGN_CLIP;
    int x, y, w, h;

    MAKE_CALLBACK(select_all);
    MAKE_CALLBACK(select_none);
    MAKE_CALLBACK(select_attach);
    MAKE_CALLBACK(dnd);

    static Fl_Menu_Item menu[] = {
        {"Select all",         0, select_all_cb,    this, FL_MENU_INACTIVE},
        {"Select none",        0, select_none_cb,   this, FL_MENU_INACTIVE},
        {"Select attachments", 0, select_attach_cb, this, FL_MENU_INACTIVE},
        {0}
    };

    /* main window */
    w = DEFAULT_W;
    h = DEFAULT_H;
    m_win = new dnd_window(w, h, default_title);
    METHODCB(m_win, quit);
    m_win->dnd(dnd_cb, this);
    m_win->dnd_enabled(true);

        /* upper area group */
        x = 0;
        y = 0;
        w = m_win->w();
        h = BUT_H + 10;
        auto g_up = new Fl_Group(x, y, w, h);

            /* "Open file" button */
            x = m_win->w() - 10 - BUT_W;
            y = 5;
            w = BUT_W;
            h = BUT_H;
            m_but_add = new Fl_Button(x, y, w, h, "Open file");
            METHODCB(m_but_add, open_file);

            /* input file label */
            x = 10;
            y = 5;
            w = m_but_add->x() - 20;
            h = BUT_H;
            m_infile_label = new Fl_Box(FL_THIN_DOWN_BOX, x, y, w, h, NULL);
            m_infile_label->labelsize(12);
            m_infile_label->align(left_align);

        g_up->end();
        g_up->resizable(m_infile_label);

        /* bottom area group */
        x = 0;
        y = m_win->h() - BUT_H*2 - 25;
        w = m_win->w();
        h = BUT_H*2 + 25;
        auto g_bttm = new Fl_Group(x, y, w, h);

            /* "Extract" button */
            x = m_win->w() - 10 - BUT_W;
            y = m_win->h() - 10 - BUT_H;
            w = BUT_W;
            h = BUT_H;
            m_but_extract = new Fl_Button(x, y, w, h, "Extract");
            m_but_extract->deactivate();
            METHODCB(m_but_extract, extract);

            /* "Abort" button */
            x = m_but_extract->x();
            y = m_but_extract->y();
            w = BUT_W;
            h = BUT_H;
            m_but_abort = new Fl_Button(x, y, w, h, "Abort");
            m_but_abort->deactivate();
            m_but_abort->hide();
            METHODCB(m_but_abort, abort);

            /* "Command" button */
            x = m_but_extract->x() - 10 - BUT_W;
            y = m_but_extract->y();
            w = BUT_W;
            h = BUT_H;
            m_but_cmd = new Fl_Button(x, y, w, h, "Command");
            m_but_cmd->deactivate();
            METHODCB(m_but_cmd, cmd);

            /* progress area group */
            x = 0;
            y = m_but_extract->y();
            w = m_win->w() - 30 - 2*BUT_W;
            h = BUT_H;
            auto g_prog = new Fl_Group(x, y, w, h);

                /* progress box */
                x = 10;
                y = g_prog->y();
                w = BUT_W;
                h = BUT_H;
                m_progress_box = new Fl_Box(FL_THIN_DOWN_BOX, x, y, w, h, NULL);
                m_progress_box->align(center_align);

                /* icon box */
                x = BUT_W + 15;
                y = g_prog->y();
                w = BUT_H;
                h = BUT_H;
                m_rotate = new rotate_box(x, y, w, h);

                /* dummy */
                x = m_but_cmd->x() - 1;
                y = g_prog->y();
                w = 1;
                h = 1;
                auto dummy = new Fl_Box(FL_NO_BOX, x, y, w, h, NULL);

            g_prog->end();
            g_prog->resizable(dummy);

            /* output directory label */
            x = 10;
            y = m_but_extract->y() - BUT_H - 5;
            w = g_prog->w() - 10;
            h = BUT_H;
            m_outdir_field = new Fl_Box(FL_THIN_DOWN_BOX, x, y, w, h, NULL);
            m_outdir_field->label(m_outdir_manual.c_str());
            m_outdir_field->align(left_align);

            /* "Source path" check button */
            x = m_but_extract->x();
            y = m_but_extract->y() - BUT_H - 5;
            w = BUT_W;
            h = BUT_H;
            m_use_source_path = new Fl_Check_Button(x, y, w, h, " Source path");
            m_use_source_path->deactivate();
            METHODCB(m_use_source_path, check_outdir);

            /* "Destination" button */
            x = m_but_cmd->x();
            y = m_use_source_path->y();
            w = BUT_W;
            h = BUT_H;
            m_but_outdir = new Fl_Button(x, y, w, h, "Destination");
            METHODCB(m_but_outdir, set_outdir);

        g_bttm->end();
        g_bttm->resizable(m_outdir_field);

        /* check browser between the two main groups */
        x = 10;
        y = g_up->y() + g_up->h();
        w = m_win->w() - 20;
        h = m_win->h() - g_up->h() - g_bttm->h();
        m_browser = new check_browser(menu, x, y, w, h);
        METHODCB(m_browser, update_browser);

    m_win->end();
    m_win->resizable(m_browser);
    m_win->size_range(MINIMUM_W, MINIMUM_H, Fl::w(), Fl::h());
}


void MKVextract::init_cmd_window()
{
    int x, y, w, h;

    /* text buffer */
    m_txtbuf = new Fl_Text_Buffer();

    /* command line window */
    w = CMD_W;
    h = CMD_H;
    m_cmd = new Fl_Double_Window(w, h, "Command line");

        /* text display */
        x = 10;
        y = 10;
        w = m_cmd->w() - 20;
        h = m_cmd->h() - 20 - BUT_H;
        auto txt = new Fl_Text_Display(x, y, w, h);
        txt->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 2);
        txt->buffer(m_txtbuf);

        /* button group */
        x = 0;
        y = txt->y() + txt->h();
        w = m_cmd->w();
        h = m_cmd->h() - y;
        auto g_bttn = new Fl_Group(x, y, w, h);

            /* "Copy" button */
            x = g_bttn->w() - 10 - BUT_W;
            y = g_bttn->y() + 5;
            w = BUT_W;
            h = BUT_H;
            auto btcopy = new Fl_Button(x, y, w, h, "Copy");
            METHODCB(btcopy, copy_command);

            /* dummy */
            x = btcopy->x() - 1;
            y = btcopy->y();
            w = 1;
            h = 1;
            auto dummy = new Fl_Box(FL_NO_BOX, x, y, w, h, NULL);

        g_bttn->end();
        g_bttn->resizable(dummy);

    m_cmd->end();
    m_cmd->resizable(txt);
    m_cmd->size_range(CMD_MIN_W, CMD_MIN_H, Fl::w(), Fl::h());
}


/* c'tor */
MKVextract::MKVextract(const char *file)
{
    /* set input file */
    m_file = absolute_str(file);

    /* set destination to current working directory */
    m_outdir_manual = get_cwd_str();

    if (!m_outdir_manual.ends_with('/')) {
        m_outdir_manual += '/';
    }

    /* multithreading */
    m_th_extract = new posix_thread (PTHREADCB(run_mkvextract), this);
    m_th_info = new posix_thread (PTHREADCB(run_mkvinfo), this);

    /* init windows */
    init_main_window();
    init_cmd_window();
}


/* d'tor */
MKVextract::~MKVextract()
{
    delete m_th_extract;
    delete m_th_info;

    delete m_cmd;
    delete m_win;
    delete m_txtbuf;
}


void MKVextract::show()
{
    if (m_win->shown()) {
        return;
    }

    m_win->show();

    /* position at desktop center */
    //m_win->wait_for_expose();
    int x = (Fl::w() - m_win->decorated_w()) / 2;
    int y = (Fl::h() - m_win->decorated_h()) / 2;
    m_win->position(x, y);

    Fl::lock();

    if (!m_file.empty()) {
        m_th_info->start();
    }
}

