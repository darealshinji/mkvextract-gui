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
#define METHODCB(WIDGET, METHOD) \
    do { \
        static auto callback_function = [] (Fl_Widget *, void *p) { \
            reinterpret_cast<decltype(this)>(p)->METHOD(); \
        }; \
        WIDGET->callback(callback_function, this); \
    } while (0)



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

    /* stop threads */
    m_th_extract->cancel();
    m_th_info->cancel();

    /* get file infos */
    m_file = std::move(str);
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
    m_fcfile->filter("*.mkv|*.mka|*.mks|*.mk3d|*.webm"); /* see https://www.matroska.org */

    if (m_fcfile->show() == 0 && (p = m_fcfile->filename()) != NULL && *p != 0) {
        /* stop threads */
        m_th_extract->cancel();
        m_th_info->cancel();

        /* get file infos */
        m_file = p;
        m_th_info->start();
    }
}


void MKVextract::do_copy_command()
{
    std::string str = m_txtbuf->text_str();
    const int len = m_txtbuf->length();
    const int destination = 1; /* 0 = selection, 1 = clipboard, 2 = both */

    /* copy entire text to clipboard */
    Fl::copy(str.c_str(), len, destination);
}


void MKVextract::do_cmd()
{
    /* set text buffer */
    std::string str = create_cmd(false);
    m_txtbuf->text(str.c_str());

    /* resize window to default and
     * position at center above main window */
    int x = m_win->x() + ((m_win->w() - m_cmdW) / 2);
    int y = m_win->y() + ((m_win->h() - m_cmdH) / 2);
    m_cmd->resize(x, y, m_cmdW, m_cmdH);
    m_cmd->show();
}


void MKVextract::do_extract()
{
    /* stop threads */
    m_th_extract->cancel();
    m_th_info->cancel();

    /* close command line window */
    m_cmd->hide();
    m_win->redraw();

    /* start extraction */
    m_th_extract->start();
}


void MKVextract::do_abort()
{
    /* stop threads */
    m_th_extract->cancel();
    m_th_info->cancel();

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
    auto menu_all = m_browser->menu(); /* "Select all" */
    auto menu_none = menu_all->next(); /* "Select none" */

    if (m_browser->nchecked() == 0) {
        m_but_extract->deactivate();
        m_but_cmd->deactivate();
        m_progress_box->label(NULL);
        menu_all->activate();
        menu_none->deactivate();
    } else {
        m_but_extract->activate();
        m_but_cmd->activate();
        m_progress_box->label("READY");
        menu_none->activate();

        if (m_browser->nchecked() == m_browser->nitems()) {
            menu_all->deactivate();
        } else {
            menu_all->activate();
        }
    }
}


void MKVextract::do_quit()
{
    /* stop threads and close all windows */
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


    /* main window */
    w = 800;
    h = 480;
    m_win = new Fl_Double_Window(w, h, "simple mkvextract GUI");
    METHODCB(m_win, do_quit);

        /* upper area group */
        x = 0;
        y = 0;
        w = m_win->w();
        h = m_btH + 10;
        auto g_up = new Fl_Group(x, y, w, h);

            /* "Open file" button */
            x = m_win->w() - 10 - m_btW;
            y = 5;
            w = m_btW;
            h = m_btH;
            m_but_add = new Fl_Button(x, y, w, h, "Open file");
            METHODCB(m_but_add, do_open_file);

            /* input file label */
            x = 11;
            y = 5;
            w = m_but_add->x() - 20;
            h = m_btH;
            m_infile_label = new Fl_Box(FL_THIN_DOWN_BOX, x, y, w, h, NULL);
            m_infile_label->label("(drag and drop a Matroska file)");
            m_infile_label->labelsize(12);
            m_infile_label->align(left_align);
            m_infile_label->deactivate();

        g_up->end();
        g_up->resizable(m_infile_label);

        /* bottom area group */
        x = 0;
        y = m_win->h() - m_btH*2 - 25;
        w = m_win->w();
        h = m_btH*2 + 25;
        auto g_bttm = new Fl_Group(x, y, w, h);

            /* "Extract" button */
            x = m_win->w() - 10 - m_btW;
            y = m_win->h() - 10 - m_btH;
            w = m_btW;
            h = m_btH;
            m_but_extract = new Fl_Button(x, y, w, h, "Extract");
            m_but_extract->deactivate();
            METHODCB(m_but_extract, do_extract);

            /* "Abort" button */
            x = m_but_extract->x();
            y = m_but_extract->y();
            w = m_btW;
            h = m_btH;
            m_but_abort = new Fl_Button(x, y, w, h, "Abort");
            m_but_abort->deactivate();
            m_but_abort->hide();
            METHODCB(m_but_abort, do_abort);

            /* "Command" button */
            x = m_but_extract->x() - 10 - m_btW;
            y = m_but_extract->y();
            w = m_btW;
            h = m_btH;
            m_but_cmd = new Fl_Button(x, y, w, h, "Command");
            m_but_cmd->deactivate();
            METHODCB(m_but_cmd, do_cmd);

            /* progress area group */
            x = 0;
            y = m_but_extract->y();
            w = m_win->w() - 30 - 2*m_btW;
            h = m_btH;
            auto g_prog = new Fl_Group(x, y, w, h);

                /* progress box */
                x = 10;
                y = g_prog->y();
                w = m_btW;
                h = m_btH;
                m_progress_box = new Fl_Box(FL_THIN_DOWN_BOX, x, y, w, h, NULL);
                m_progress_box->align(center_align);

                /* icon box */
                x = m_btW + 15;
                y = g_prog->y();
                w = m_btH;
                h = m_btH;
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
            y = m_but_extract->y() - m_btH - 5;
            w = g_prog->w() - 10;
            h = m_btH;
            m_outdir_field = new Fl_Box(FL_THIN_DOWN_BOX, x, y, w, h, NULL);
            m_outdir_field->label(m_outdir_manual.c_str());
            m_outdir_field->align(left_align);

            /* "Source path" check button */
            x = m_but_extract->x();
            y = m_but_extract->y() - m_btH - 5;
            w = m_btW;
            h = m_btH;
            m_use_source_path = new Fl_Check_Button(x, y, w, h, " Source path");
            m_use_source_path->deactivate();
            METHODCB(m_use_source_path, do_check_outdir);

            /* "Destination" button */
            x = m_but_cmd->x();
            y = m_use_source_path->y();
            w = m_btW;
            h = m_btH;
            m_but_outdir = new Fl_Button(x, y, w, h, "Destination");
            METHODCB(m_but_outdir, do_set_outdir);

        g_bttm->end();
        g_bttm->resizable(m_outdir_field);

        /* check browser between the two main groups */
        x = 10;
        y = g_up->y() + g_up->h();
        w = m_win->w() - 20;
        h = m_win->h() - g_up->h() - g_bttm->h();
        m_browser = new check_browser(menu, x, y, w, h);
        METHODCB(m_browser, do_update_browser);

        /* drag 'n drop area covering entire window */
        x = 0;
        y = 0;
        w = m_win->w();
        h = m_win->h();
        m_dnd_area = new dnd_box(x, y, w, h);
        METHODCB(m_dnd_area, do_dnd);

    m_win->end();
    m_win->resizable(m_browser);

    w = m_win->w() / 2;
    h = m_win->h() / 2;
    m_win->size_range(w, h, Fl::w(), Fl::h());

    /* position at desktop center */
    x = (Fl::w() - m_win->decorated_w()) / 2;
    y = (Fl::h() - m_win->decorated_h()) / 2;
    m_win->position(x, y);
}


void MKVextract::init_cmd_window()
{
    int x, y, w, h;

    /* text buffer */
    m_txtbuf = new Fl_Text_Buffer();

    /* command line window */
    w = m_cmdW;
    h = m_cmdH;
    m_cmd = new Fl_Double_Window(w, h, "Command line");

        /* text display */
        x = 10;
        y = 10;
        w = m_cmd->w() - 20;
        h = m_cmd->h() - 20 - m_btH;
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
            x = g_bttn->w() - 10 - m_btW;
            y = g_bttn->y() + 5;
            w = m_btW;
            h = m_btH;
            auto btcopy = new Fl_Button(x, y, w, h, "Copy");
            METHODCB(btcopy, do_copy_command);

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

    w = m_cmd->w() / 2;
    h = m_cmd->h() / 2;
    m_cmd->size_range(w, h, Fl::w(), Fl::h());
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

    /* init windows */
    init_main_window();
    init_cmd_window();

    /* native file choosers */
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

