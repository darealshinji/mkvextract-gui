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
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>

#include <string>
#include <utility>

#include "check_browser.hpp"
#include "dnd.hpp"
#include "posix_thread.hpp"
#include "rotate_box.hpp"
#include "mkvextract.hpp"



/* inline lambda start routine for pthread_create() */
#define PTHREADCB(METHOD) \
    [] (void *p) -> void* { \
        reinterpret_cast<decltype(this)>(p)->METHOD(); \
        return NULL; \
    }


/* inline lambda callback function for Fl_Menu_Item */
#define MENUCB(METHOD) \
    [] (Fl_Widget *, void *p) { \
        reinterpret_cast<decltype(this)>(p)->METHOD(); \
    }


/* generate callback function and set callback */
#define METHODCB(WIDGET, OBJECT, METHOD) \
    do { \
        static auto callback_function = [] (Fl_Widget *, void *p) { \
            reinterpret_cast<decltype(OBJECT)>(p)->METHOD(); \
        }; \
        WIDGET->callback(callback_function, OBJECT); \
    } while (0)



static inline void position_at_center(Fl_Double_Window *o)
{
    o->position((Fl::w() - o->decorated_w()) / 2,
                (Fl::h() - o->decorated_h()) / 2);
}


void MKVextract::restore_main_window()
{
    m_but_outdir->activate();
    m_but_add->activate();

    m_but_abort->deactivate();
    m_but_abort->hide();
    m_but_extract->activate();
    m_but_extract->show();

    m_dnd_area->activate();
    m_use_source_path->activate();
    m_rotate->stop();
}


void MKVextract::do_dnd()
{
    const char *text = Fl::event_text();

    /* unlikely but it doesn't hurt to check */
    if (!text || !*text) {
        return;
    }

    /* find newline separator */
    const char *p = strchr(text, '\n');

    if (!p) {
        return;
    }

    /* copy first item, without newline */
    std::string s(text, p - text);

    if (s.starts_with("file://")) {
        /* decode URI in place */
        s.erase(0, 7);
        fl_decode_uri(std::data(s));
    }

    if (!s.starts_with('/')) {
        /* not a full path */
        return;
    }

    /* stop threads */
    m_th_extract->cancel();
    m_th_info->cancel();

    m_file = std::move(s);
    m_th_info->start();
}


void MKVextract::do_set_outdir()
{
    const char *p;

    m_fcdir->title("Select output directory");

    if (m_fcdir->show() == 0 && (p = m_fcdir->filename()) != NULL && *p != 0) {
        m_outdir_manual = p;

        if (!m_outdir_manual.ends_with('/')) {
            m_outdir_manual += '/';
        }

        m_outdir_field->label(m_outdir_manual.c_str());
        m_outdir_field->activate();
        m_use_source_path->clear();
    }
}


void MKVextract::do_open_file()
{
    const char *p;

    m_fcfile->title("Select a file");
    m_fcfile->filter("*.mkv|*.mka|*.mks|*.mk3d|*.webm"); /* https://www.matroska.org */

    if (m_fcfile->show() == 0 && (p = m_fcfile->filename()) != NULL && *p != 0) {
        /* stop running threads */
        m_th_extract->cancel();
        m_th_info->cancel();

        m_file = p;
        m_th_info->start();
    }
}


void MKVextract::do_copy_command()
{
    Fl::copy(m_txtbuf->text_str().c_str(), m_txtbuf->length(), 1);
}


void MKVextract::do_cmd()
{
    m_txtbuf->text(create_cmd(false).c_str());
    m_cmd->show();
}


void MKVextract::do_extract()
{
    /* stop running threads */
    m_th_extract->cancel();
    m_th_info->cancel();

    m_cmd->hide();
    m_win->redraw();

    m_th_extract->start();
}


void MKVextract::do_abort()
{
    m_th_extract->cancel();
    m_th_info->cancel();
    m_progress_box->label("STOPPED");
    restore_main_window();
}


void MKVextract::do_check_outdir()
{
    if (m_use_source_path->value() == true) {
        m_outdir_field->label(m_outdir_source.c_str());
        m_outdir_field->deactivate();
    } else {
        m_outdir_field->label(m_outdir_manual.c_str());
        m_outdir_field->activate();
    }
}


void MKVextract::do_update_browser()
{
    if (m_browser->nchecked() > 0) {
        m_but_extract->activate();
        m_but_cmd->activate();
        m_progress_box->label("READY");
    } else {
        m_but_extract->deactivate();
        m_but_cmd->deactivate();
        m_progress_box->label(NULL);
    }
}


void MKVextract::do_quit()
{
    m_th_extract->cancel();
    m_th_info->cancel();
    Fl::hide_all_windows();
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


void MKVextract::init_main_window()
{
    const int center_align = FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP;
    const int left_align   = FL_ALIGN_LEFT   | FL_ALIGN_INSIDE | FL_ALIGN_CLIP;
    int x, y, w, h;

    static Fl_Menu_Item menu[] = {
        {"Select all",  0, MENUCB(do_select_all),  this, FL_MENU_INACTIVE},
        {"Select none", 0, MENUCB(do_select_none), this, FL_MENU_INACTIVE|FL_MENU_DIVIDER},
        {"Open file",   0, MENUCB(do_open_file),   this},
        {"Quit",        0, MENUCB(do_quit),        this},
        {0}
    };

    m_win = new Fl_Double_Window(800, 480, "simple mkvextract GUI");
    METHODCB(m_win, this, do_quit);

        /* upper area group */
        h = m_btH + 5;
        auto g_up = new Fl_Group(0, 0, m_win->w(), h);

            /* "Open file" button */
            x = m_win->w() - 10 - m_btW;
            m_but_add = new Fl_Button(x, 5, m_btW, m_btH, "Open file");
            METHODCB(m_but_add, this, do_open_file);

            /* input file label */
            w = m_but_add->x() - 20;
            m_infile_label = new Fl_Box(FL_THIN_DOWN_BOX, 11, 5, w, m_btH, NULL);
            m_infile_label->label("(drag and drop a Matroska file)");
            m_infile_label->labelsize(12);
            m_infile_label->align(left_align);
            m_infile_label->deactivate();

        g_up->end();
        g_up->resizable(m_infile_label);

        /* bottom area group */
        y = m_win->h() - m_btH*2 - 25;
        w = m_win->w();
        h = m_btH*2 + 25;
        auto g_bttm = new Fl_Group(0, y, w, h);

            /* "Extract" button */
            x = m_win->w() - 10 - m_btW;
            y = m_win->h() - 10 - m_btH;
            m_but_extract = new Fl_Button(x, y, m_btW, m_btH, "Extract");
            m_but_extract->deactivate();
            METHODCB(m_but_extract, this, do_extract);

            /* "Abort" button */
            x = m_but_extract->x();
            y = m_but_extract->y();
            m_but_abort = new Fl_Button(x, y, m_btW, m_btH, "Abort");
            m_but_abort->deactivate();
            m_but_abort->hide();
            METHODCB(m_but_abort, this, do_abort);

            /* "Command" button */
            x = m_but_extract->x() - 10 - m_btW;
            y = m_but_extract->y();
            m_but_cmd = new Fl_Button(x, y, m_btW, m_btH, "Command");
            m_but_cmd->deactivate();
            METHODCB(m_but_cmd, this, do_cmd);

            /* progress area group */
            y = m_but_extract->y();
            w = m_win->w() - 30 - 2*m_btW;
            auto g_prog = new Fl_Group(0, y, w, m_btH);

                /* progress box */
                y = m_but_extract->y();
                m_progress_box = new Fl_Box(FL_THIN_DOWN_BOX, 10, y, m_btW, m_btH, NULL);
                m_progress_box->align(center_align);

                /* icon box */
                x = m_btW + 15;
                y = m_progress_box->y();
                m_rotate = new rotate_box(x, y, m_btH, m_btH);

                /* dummy */
                x = m_but_cmd->x() - 1;
                y = m_progress_box->y();
                auto dummy1 = new Fl_Box(FL_NO_BOX, x, y, 1, 1, NULL);

            g_prog->end();
            g_prog->resizable(dummy1);

            /* output directory group */
            auto g_outd = new Fl_Group(0, g_bttm->y(), g_prog->w(), m_btH);
            g_outd->begin();

                /* output directory label */
                y = m_but_extract->y() - m_btH - 5;
                w = g_outd->w() - 10;
                m_outdir_field = new Fl_Box(FL_THIN_DOWN_BOX, 10, y, w, m_btH, NULL);
                m_outdir_field->label(m_outdir_manual.c_str());
                m_outdir_field->align(left_align);

            g_outd->end();
            g_outd->resizable(m_outdir_field);

            /* "Source path" check button */
            x = m_but_extract->x();
            y = m_but_extract->y() - m_btH - 5;
            m_use_source_path = new Fl_Check_Button(x, y, m_btW, m_btH, " Source path");
            m_use_source_path->deactivate();
            METHODCB(m_use_source_path, this, do_check_outdir);

            /* "Destination" button */
            x = m_but_cmd->x();
            y = m_use_source_path->y();
            m_but_outdir = new Fl_Button(x, y, m_btW, m_btH, "Destination");
            METHODCB(m_but_outdir, this, do_set_outdir);

        g_bttm->end();
        g_bttm->resizable(g_outd);

        /* check browser */
        y = m_but_add->y() + m_but_add->h() + 5;
        w = m_win->w() - 20;
        h = m_win->h() - m_btH*3 - 35;
        m_browser = new check_browser(menu, 10, y, w, h);
        METHODCB(m_browser, this, do_update_browser);

        /* drag 'n drop area */
        m_dnd_area = new dnd_box(0, 0, m_win->w(), m_win->h());
        METHODCB(m_dnd_area, this, do_dnd);

    m_win->end();
    m_win->resizable(m_browser);
    m_win->size_range(512, 384, Fl::w(), Fl::h());
    position_at_center(m_win);
}


void MKVextract::init_cmd_window()
{
    int x, y, w, h;

    m_cmd = new Fl_Double_Window(640, 320, "Command line");

        /* text display */
        w = m_cmd->w() - 30;
        h = m_cmd->h() - 30 - m_btH;
        auto txt = new Fl_Text_Display(15, 15, w, h);
        txt->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 2);

        /* button group */
        y = txt->h() + txt->y();
        w = m_cmd->w();
        h = m_cmd->h() - txt->h() - txt->y();
        auto g_bttn = new Fl_Group(0, y, w, h);
        g_bttn->begin();

            /* "Copy" button */
            x = m_cmd->w() - 110 - 15;
            y = txt->h() + txt->y() + 6;
            auto btcopy = new Fl_Button(x, y, 110, m_btH, "Copy");
            METHODCB(btcopy, this, do_copy_command);

            /* dummy */
            x = btcopy->x() - 1;
            y = btcopy->y();
            auto dummy = new Fl_Box(FL_NO_BOX, x, y, 1, 1, NULL);

        g_bttn->end();
        g_bttn->resizable(dummy);

    m_cmd->end();
    m_cmd->resizable(txt);
    m_cmd->size_range(m_cmd->w(), m_cmd->h(), Fl::w(), Fl::h());
    position_at_center(m_cmd);

    /* text buffer */
    m_txtbuf = new Fl_Text_Buffer();
    txt->buffer(m_txtbuf);
}


/* c'tor */
MKVextract::MKVextract(const char *file)
{
    /* set input file */
    if (file && *file) {
        m_file = fl_filename_absolute_str(file);
    }

    /* set destination to current working directory */
    m_outdir_manual = fl_getcwd_str();

    if (m_outdir_manual.empty()) {
        m_outdir_manual = "/tmp/";
    } else if (!m_outdir_manual.ends_with('/')) {
        m_outdir_manual += '/';
    }

    /* multithreading */
    m_th_extract = new posix_thread (PTHREADCB(run_mkvextract), this);
    m_th_info = new posix_thread (PTHREADCB(run_mkvinfo), this);

    /* main window */
    init_main_window();

    /* command line window */
    init_cmd_window();

    /* file choosers */
    m_fcdir = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    m_fcfile = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);

}


/* d'tor */
MKVextract::~MKVextract()
{
    /* threads */
    delete m_th_extract;
    delete m_th_info;

    /* main widgets */
    delete m_cmd;
    delete m_win;
    delete m_txtbuf;

    /* file choosers */
    delete m_fcdir;
    delete m_fcfile;
}


void MKVextract::show()
{
    if (m_win->shown()) {
        return;
    }

    m_win->show();

    /* test timeout handler */
    //m_rotate->start();

    Fl::lock();

    if (!m_file.empty()) {
        m_th_info->start();
    }
}

