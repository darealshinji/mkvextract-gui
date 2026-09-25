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

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Double_Window.H>
#include <FL/filename.H>
#include <fontconfig/fontconfig.h>

#include <cstdlib>
#include <array>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dnd.hpp"
#include "check_browser.hpp"
#include "pipe_command.hpp"
#include "posix_thread.hpp"
#include "rotate.hpp"
#include "mkvextract.hpp"



static inline void position_at_center(Fl_Double_Window *o)
{
    o->position((Fl::w() - o->decorated_w()) / 2,
                (Fl::h() - o->decorated_h()) / 2);
}


void MKVextract::restore_main_window()
{
    m_but_outdir->activate();
    m_but_add->activate();
    m_but_extract->label("Extract");
    m_but_extract->callback(extract_cb, this);

    m_dnd_area->activate();
    m_use_source_path->activate();
    m_rotate->deactivate();
}


/* callback functions */

void MKVextract::dnd_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->do_dnd();
}

void MKVextract::do_dnd()
{
    std::string items(Fl::event_text());
    size_t pos = items.find('\n');

    if (pos != std::string::npos) {
        items.erase(pos);

        if (items.starts_with("file:///")) {
            /* URI */
            char *copy = strdup(items.c_str());
            fl_decode_uri(copy);
            m_file = copy + 7;
            free(copy);
            m_th_info->start();
        } else if (items.starts_with('/')) {
            m_file = items;
            m_th_info->start();
        }
    }
}


void MKVextract::browse_outdir_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->do_browse_outdir();
}

void MKVextract::do_browse_outdir()
{
    const char *p;

    m_fcdir->title("Select output directory");

    if (m_fcdir->show() == 0 && (p = m_fcdir->filename()) != NULL && *p != 0) {
        m_outdir_manual = p;

        if (!m_outdir_manual.ends_with('/')) {
            m_outdir_manual += '/';
        }

        m_outdir_field->copy_label(m_outdir_manual.c_str());
        m_outdir_field->activate();
        m_use_source_path->clear();
    }
}


void MKVextract::add_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->do_add();
}

void MKVextract::do_add()
{
    const char *p;

    m_fcfile->title("Select a file");
    m_fcfile->filter("*.mkv|*.mka|*.mks|*.mk3d|*.webm"); /* https://www.matroska.org */

    if (m_fcfile->show() == 0 && (p = m_fcfile->filename()) != NULL && *p != 0) {
        m_file = p;
        m_th_info->start();
    }
}


void MKVextract::clipboard_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->do_clipboard();
}

void MKVextract::do_clipboard()
{
    char *text = m_txtbuf->text();
    Fl::copy(text, m_txtbuf->length(), 1);
    free(text);
}


void MKVextract::cmd_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->do_cmd();
}

void MKVextract::do_cmd()
{
    std::string command = create_cmd(false);
    m_txtbuf->text(command.c_str());
    m_cmd->show();
}


void MKVextract::extract_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->do_extract();
}

void MKVextract::do_extract()
{
    m_cmd->hide();
    m_win->redraw();
    m_th_extract->start();
}


void MKVextract::abort_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->do_abort();
}

void MKVextract::do_abort()
{
    m_th_extract->cancel();
    m_th_info->cancel();
    m_progress_box->label("STOPPED");
    restore_main_window();
}


void MKVextract::check_outdir_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->do_check_outdir();
}

void MKVextract::do_check_outdir()
{
    if (m_use_source_path->value() == true) {
        m_outdir_field->copy_label(m_outdir_source.c_str());
        m_outdir_field->deactivate();
    } else {
        m_outdir_field->copy_label(m_outdir_manual.c_str());
        m_outdir_field->activate();
    }
}


void MKVextract::update_browser_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->do_update_browser();
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


void MKVextract::close_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->do_close();
}

void MKVextract::do_close()
{
    m_th_extract->cancel();
    m_th_info->cancel();
    Fl::hide_all_windows();
}


void MKVextract::close_cmd_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->m_cmd->hide();
}


void MKVextract::select_all_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->do_select_all();
}

void MKVextract::do_select_all()
{
    m_browser->check_all();
    do_update_browser();
}


void MKVextract::select_none_cb(Fl_Widget *, void *p) {
    reinterpret_cast<MKVextract *>(p)->do_select_none();
}

void MKVextract::do_select_none()
{
    m_browser->check_none();
    do_update_browser();
}


void MKVextract::dismiss_cb(Fl_Widget *, void *) {
}


/* c'tor */
MKVextract::MKVextract()
{
    const int bt_h = 28;
    const int bt_w = 110;
    const int center_align = FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP;
    const int left_align = FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP;
    int x, y, w, h;


    /* set destination to current working directory */
    m_outdir_manual = fl_getcwd_str();

    if (m_outdir_manual.empty()) {
        m_outdir_manual = "/tmp/";
    } else if (!m_outdir_manual.ends_with('/')) {
        m_outdir_manual += '/';
    }


    /* threads */
    m_th_info = new posix_thread(thread_run_mkvinfo, this);
    m_th_extract = new posix_thread(thread_run_mkvextract, this);


    /* init fontconfig */
    FcInit();


    /* use mkvextract icon if present */
    const std::array<const char *, 6> paths = {
        "/usr/share/icons/hicolor/256x256/apps/mkvextract.png",
        "/usr/share/icons/hicolor/128x128/apps/mkvextract.png",
        "/usr/share/icons/hicolor/64x64/apps/mkvextract.png",
        "/usr/share/icons/hicolor/48x48/apps/mkvextract.png",
        "/usr/share/icons/hicolor/32x32/apps/mkvextract.png",
        "/usr/share/pixmaps/mkvextract.png"
    };

    for (const char *e : paths) {
        Fl_PNG_Image icon(e);

        if (!icon.fail()) {
            Fl_Window::default_icon(&icon);
            break;
        }
    }


    static Fl_Menu_Item menu[] = {
        /* text, shortcut, callback, user data, label type */
        { " Select all",     0, select_all_cb,  this, FL_MENU_INACTIVE                   },
        { " Select none",    0, select_none_cb, this, FL_MENU_INACTIVE | FL_MENU_DIVIDER },
        { " Open file",      0, add_cb,         this                                     },
        { " Close program ", 0, close_cb,       this, FL_MENU_DIVIDER                    },
        { " Dismiss",        0, dismiss_cb,     NULL                                     },
        { 0 }
    };


    /* main window */
    m_win = new Fl_Double_Window(800, 480, "simple mkvextract GUI");
    m_win->callback(close_cb, this);

        /* upper area group */
        h = bt_h + 5;
        auto g_up = new Fl_Group(0, 0, m_win->w(), h);

            /* "Open file" button */
            x = m_win->w() - 10 - bt_w;
            m_but_add = new Fl_Button(x, 5, bt_w, bt_h, "Open file");
            m_but_add->callback(add_cb, this);

            /* input file label */
            w = m_but_add->x() - 20;
            m_infile_label = new Fl_Box(FL_THIN_DOWN_BOX, 11, 5, w, bt_h, NULL);
            m_infile_label->label("(drag and drop a Matroska file)");
            m_infile_label->labelsize(12);
            m_infile_label->align(left_align);
            m_infile_label->deactivate();

        g_up->end();
        g_up->resizable(m_infile_label);

        /* bottom area group */
        y = m_win->h() - bt_h*2 - 25;
        w = m_win->w();
        h = bt_h*2 + 25;
        auto g_bttm = new Fl_Group(0, y, w, h);

            /* "Extract" button */
            x = m_win->w() - 10 - bt_w;
            y = m_win->h() - 10 - bt_h;
            m_but_extract = new Fl_Button(x, y, bt_w, bt_h, "Extract");
            m_but_extract->callback(extract_cb, this);
            m_but_extract->deactivate();

            /* "Command" button */
            x = m_but_extract->x() - 10 - bt_w;
            y = m_but_extract->y();
            m_but_cmd = new Fl_Button(x, y, bt_w, bt_h, "Command");
            m_but_cmd->callback(cmd_cb, this);
            m_but_cmd->deactivate();

            /* progress area group */
            y = m_but_extract->y();
            w = m_win->w() - 30 - 2*bt_w;
            auto g_prog = new Fl_Group(0, y, w, bt_h);

                /* progress box */
                y = m_but_extract->y();
                m_progress_box = new Fl_Box(FL_THIN_DOWN_BOX, 10, y, bt_w, bt_h, NULL);
                m_progress_box->align(center_align);

                /* icon box */
                x = bt_w + 15;
                y = m_progress_box->y();
                auto box_rotating_icon = new Fl_Box(FL_NO_BOX, x, y, bt_h, bt_h, NULL);

                /* dummy */
                x = m_but_cmd->x() - 1;
                y = m_progress_box->y();
                auto dummy1 = new Fl_Box(FL_NO_BOX, x, y, 1, 1, NULL);

            g_prog->end();
            g_prog->resizable(dummy1);

            /* output directory group */
            auto g_outd = new Fl_Group(0, g_bttm->y(), g_prog->w(), bt_h);
            g_outd->begin();

                /* output directory label */
                y = m_but_extract->y() - bt_h - 5;
                w = g_outd->w() - 10;
                m_outdir_field = new Fl_Box(FL_THIN_DOWN_BOX, 10, y, w, bt_h, NULL);
                m_outdir_field->label(m_outdir_manual.c_str());
                m_outdir_field->align(left_align);

            g_outd->end();
            g_outd->resizable(m_outdir_field);

            /* "Source path" check button */
            x = m_but_extract->x();
            y = m_but_extract->y() - bt_h - 5;
            m_use_source_path = new Fl_Check_Button(x, y, bt_w, bt_h, " Source path");
            m_use_source_path->deactivate();
            m_use_source_path->callback(check_outdir_cb, this);

            /* "Destination" button */
            x = m_but_cmd->x();
            y = m_use_source_path->y();
            m_but_outdir = new Fl_Button(x, y, bt_w, bt_h, "Destination");
            m_but_outdir->callback(browse_outdir_cb, this);

        g_bttm->end();
        g_bttm->resizable(g_outd);

        /* check browser */
        y = m_but_add->y() + m_but_add->h() + 5;
        w = m_win->w() - 20;
        h = m_win->h() - bt_h*3 - 35;
        m_browser = new check_browser(10, y, w, h);
        m_browser->callback(update_browser_cb, this);
        m_browser->menu(menu);

        /* drag 'n drop area */
        m_dnd_area = new dnd_box(0, 0, m_win->w(), m_win->h());
        m_dnd_area->callback(dnd_cb, this);

    m_win->end();
    m_win->resizable(m_browser);
    m_win->size_range(512, 384, Fl::w(), Fl::h());
    position_at_center(m_win);


    /* Command line window */
    m_cmd = new Fl_Double_Window(640, 320, "Command line");

        /* text display */
        w = m_cmd->w() - 30;
        h = m_cmd->h() - 30 - bt_h;
        auto txt = new Fl_Text_Display(15, 15, w, h);
        txt->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 2);

        /* button group */
        y = txt->h() + txt->y();
        w = m_cmd->w();
        h = m_cmd->h() - txt->h() - txt->y();
        auto g_bttn = new Fl_Group(0, y, w, h);
        g_bttn->begin();

            /* "Close" button */
            x = m_cmd->w() - 110 - 15;
            y = txt->h() + txt->y() + 6;
            auto btclose = new Fl_Button(x, y, 110, bt_h, "Close");
            btclose->callback(close_cmd_cb, this);

            /* "Copy to clipboard" button */
            x = btclose->x() - 150 - 5;
            y = btclose->y();
            auto btcopy = new Fl_Button(x, y, 150, bt_h, "Copy to clipboard");
            btcopy->callback(clipboard_cb, this);

            /* dummy */
            x = btcopy->x() - 1;
            y = btcopy->y();
            auto dummy2 = new Fl_Box(FL_NO_BOX, x, y, 1, 1, NULL);

        g_bttn->end();
        g_bttn->resizable(dummy2);

    m_cmd->end();
    m_cmd->resizable(txt);
    m_cmd->size_range(m_cmd->w(), m_cmd->h(), Fl::w(), Fl::h());
    position_at_center(m_cmd);


    /* file choosers */
    m_fcdir = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    m_fcfile = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);


    /* text buffer */
    m_txtbuf = new Fl_Text_Buffer();
    txt->buffer(m_txtbuf);


    /* rotating icon */
    m_rotate = new rotate(box_rotating_icon);
}


/* d'tor */
MKVextract::~MKVextract()
{
    delete m_cmd;
    delete m_win;
    delete m_txtbuf;
    delete m_fcdir;
    delete m_fcfile;
    delete m_rotate;
}


void MKVextract::show(const char *file)
{
    if (m_win->shown()) {
        return;
    }

    m_win->show();

    /* test timeout handler */
    //m_rotate->activate();

    if (file && *file) {
        if (fl_filename_isdir(file)) {
            fl_message_title("Error");
            fl_message("`%s' is a directory!", file);
        } else if (access(file, R_OK) != 0) {
            fl_message_title("Error");
            fl_message("cannot read file `%s'", file);
        } else {
            m_file = fl_filename_absolute_str(file);
        }
    }

    Fl::lock();

    if (!m_file.empty()) {
        m_th_info->start();
    }
}

