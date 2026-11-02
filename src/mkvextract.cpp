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
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>

#include <filesystem>
#include <semaphore>
#include <sstream>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>

#include "check_browser.hpp"
#include "dnd_window.hpp"
#include "fnfc.hpp"
#include "rotate_box.hpp"
#include "mkvextract.hpp"

#define VENDOR        "djcj"
#define APPNAME       "simple-mkvextract-gui"
#define DEFAULT_TITLE "simple mkvextract GUI"



/* multithreading */

static bool semaphore_in_use(std::binary_semaphore &sem)
{
    /* try_acquire() fails without blocking if semaphore
     * is still in use by the extraction thread */
    if (sem.try_acquire()) {
        sem.release();
        return false;
    }

    return true;
}

void MKVextract::start_info_thread(const char *file)
{
    stop_and_delete_threads();

    m_file = fs::absolute(file);

    m_th_info = new std::jthread([this] (std::stop_token stoken) {
        run_mkvinfo(stoken);
    });
}


void MKVextract::start_extract_thread()
{
    stop_and_delete_threads();

    m_th_extract = new std::jthread([this] (std::stop_token stoken) {
        m_extract_semaphore.acquire();
        run_mkvextract(stoken);
        m_extract_semaphore.release();
    });
}


/* automatic stop and join through destructors */
void MKVextract::stop_and_delete_threads()
{
    if (m_th_extract) {
        delete m_th_extract;
        m_th_extract = NULL;
    }

    if (m_th_info) {
        delete m_th_info;
        m_th_info = NULL;
    }
}


void MKVextract::restore_main_window()
{
    m_box_rotate->stop();

    m_but_outdir->activate();
    m_but_open->activate();

    m_but_abort->deactivate();
    m_but_abort->hide();

    m_but_extract->activate();
    m_but_extract->show();

    m_chk_sourcepath->activate();
    do_check_outdir();

    m_win_main->dnd_enabled(true);
    m_win_main->redraw();
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
    std::string item(text, p - text);

    if (item.starts_with("file://")) {
        item.erase(0, 7);
        fl_decode_uri(std::data(item));
    }

    if (!item.empty()) {
        start_info_thread(item.c_str());
    }
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
    m_box_outdir->copy_label(m_outdir_manual.c_str());
    m_box_outdir->activate();
    m_chk_sourcepath->clear();
}


void MKVextract::do_open_file()
{
    FNFC fc(FNFC::BROWSE_FILE, "Select a file");
    fc.filter("Matroska\t*.mkv,*.mka,*.mks,*.mk3d,*.webm");

    const char *file = fc.select();

    if (file && *file) {
        start_info_thread(file);
    }
}


void MKVextract::do_cmd()
{
    create_cmd(CREATE_TEXTBUFFER);

    /* resize window if it was reopened */
    if (!m_win_cmd->shown()) {
        int x, y;

        if (m_chk_center->value() == 0) {
            x = m_win_cmd->x();
            y = m_win_cmd->y();
        } else {
            x = m_win_main->x() + ((m_win_main->w() - CMD_W) / 2);
            y = m_win_main->y() + ((m_win_main->h() - CMD_H) / 2);
        }

        m_win_cmd->resize(x, y, CMD_W, CMD_H);
    }

    m_win_cmd->show();
}


void MKVextract::do_settings()
{
    if (!m_win_set->shown() && m_chk_center->value() != 0) {
        /* position at center above main window */
        int x = m_win_main->x() + ((m_win_main->w() - m_win_set->w()) / 2);
        int y = m_win_main->y() + ((m_win_main->h() - m_win_set->h()) / 2);
        m_win_set->position(x, y);
    }

    m_win_set->show();
}


void MKVextract::do_extract()
{
    m_win_cmd->hide();
    m_win_set->hide();
    start_extract_thread();
}


void MKVextract::do_abort()
{
    stop_and_delete_threads();
    m_box_progress->label("STOPPED");
    restore_main_window();
}


void MKVextract::do_check_outdir()
{
    if (m_chk_sourcepath->value() == 0) {
        /* manually selected output directory */
        m_box_outdir->copy_label(m_outdir_manual.c_str());
        m_box_outdir->activate();
    } else {
        /* same as input file directory */
        m_box_outdir->copy_label(m_outdir_source.c_str());
        m_box_outdir->deactivate();
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
        m_box_progress->label(NULL);

        sel_none->deactivate();
        sel_all->activate();
        return;
    }

    /* one or more items selected */
    m_but_extract->activate();
    m_but_cmd->activate();
    m_box_progress->label("READY");

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

    /* update command window */
    if (m_win_cmd->shown()) {
        create_cmd(CREATE_TEXTBUFFER);
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
    if (semaphore_in_use(m_extract_semaphore)) {
        const char *msg = "Extraction is still in progress.\nClose the program anyway?";

        fl_message_title(DEFAULT_TITLE);

        if (fl_choice("%s", "Cancel", "Close", "Don't close", msg) != 1) {
            return;
        }
    }

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

    static Fl_SVG_Image svg(NULL, svg::settings, svg::settings_len);

    static Fl_Menu_Item menu[] = {
        {"Select all",         0, select_all_cb,    this, FL_MENU_INACTIVE},
        {"Select none",        0, select_none_cb,   this, FL_MENU_INACTIVE},
        {"Select attachments", 0, select_attach_cb, this, FL_MENU_INACTIVE},
        {0}
    };

    /* main window */
    w = DEFAULT_W;
    h = DEFAULT_H;
    m_win_main = new dnd_window(w, h, DEFAULT_TITLE);
    m_win_main->dnd(dnd_cb, this);
    m_win_main->dnd_enabled(true);
    METHODCB(m_win_main, quit);

        /* upper area group */
        x = 0;
        y = 0;
        w = m_win_main->w();
        h = BUT_H + 10;
        auto g_up = new Fl_Group(x, y, w, h);

            /* "Settings" button */
            x = m_win_main->w() - 10 - BUT_H;
            y = 5;
            w = BUT_H;
            h = BUT_H;
            auto but_set = new Fl_Button(x, y, w, h);
            svg.scale(w-4, h-4, 0, 1);
            but_set->image(svg);
            but_set->tooltip("Settings");
            METHODCB(but_set, settings);

            /* "Open file" button */
            x = but_set->x() - 10 - BUT_W;
            y = 5;
            w = BUT_W;
            h = BUT_H;
            m_but_open = new Fl_Button(x, y, w, h, "Open file");
            METHODCB(m_but_open, open_file);

            /* input file label */
            x = 10;
            y = 5;
            w = m_but_open->x() - 20;
            h = BUT_H;
            m_box_infile = new Fl_Box(FL_THIN_DOWN_BOX, x, y, w, h, NULL);
            m_box_infile->align(left_align);

        g_up->end();
        g_up->resizable(m_box_infile);

        /* bottom area group */
        x = 0;
        y = m_win_main->h() - BUT_H*2 - 25;
        w = m_win_main->w();
        h = BUT_H*2 + 25;
        auto g_bttm = new Fl_Group(x, y, w, h);

            /* "Extract" button */
            x = m_win_main->w() - 10 - BUT_W;
            y = m_win_main->h() - 10 - BUT_H;
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
            w = m_win_main->w() - 30 - 2*BUT_W;
            h = BUT_H;
            auto g_prog = new Fl_Group(x, y, w, h);

                /* progress box */
                x = 10;
                y = g_prog->y();
                w = BUT_W;
                h = BUT_H;
                m_box_progress = new Fl_Box(FL_THIN_DOWN_BOX, x, y, w, h, NULL);
                m_box_progress->align(center_align);

                /* icon box */
                x = BUT_W + 15;
                y = g_prog->y();
                w = BUT_H;
                h = BUT_H;
                m_box_rotate = new rotate_box(x, y, w, h);

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
            m_box_outdir = new Fl_Box(FL_THIN_DOWN_BOX, x, y, w, h, NULL);
            m_box_outdir->copy_label(m_outdir_manual.c_str());
            m_box_outdir->align(left_align);

            /* "Source path" check button */
            x = m_but_extract->x();
            y = m_but_extract->y() - BUT_H - 5;
            w = BUT_W;
            h = BUT_H;
            m_chk_sourcepath = new Fl_Check_Button(x, y, w, h, " Source path");
            m_chk_sourcepath->deactivate();
            METHODCB(m_chk_sourcepath, check_outdir);

            /* "Destination" button */
            x = m_but_cmd->x();
            y = m_chk_sourcepath->y();
            w = BUT_W;
            h = BUT_H;
            m_but_outdir = new Fl_Button(x, y, w, h, "Destination");
            METHODCB(m_but_outdir, set_outdir);

        g_bttm->end();
        g_bttm->resizable(m_box_outdir);

        /* check browser between the two main groups */
        x = 10;
        y = g_up->y() + g_up->h();
        w = m_win_main->w() - 20;
        h = m_win_main->h() - g_up->h() - g_bttm->h();
        m_browser = new check_browser(menu, x, y, w, h);
        m_browser->box(FL_THIN_DOWN_BOX);
        m_browser->color(fl_lighter(FL_BACKGROUND_COLOR));
        METHODCB(m_browser, update_browser);

    m_win_main->end();
    m_win_main->resizable(m_browser);
    m_win_main->size_range(MINIMUM_W, MINIMUM_H, Fl::w(), Fl::h());
}


/* c'tor */
MKVextract::MKVextract()
: m_extract_semaphore(1),
  m_th_extract(NULL),
  m_th_info(NULL)
{
    std::string pref_scheme;
    int pref_theme, pref_center;

    /* get settings */
    Fl_Preferences pref(Fl_Preferences::USER_L, VENDOR, APPNAME);
    pref.get("scheme", pref_scheme, "base");
    pref.get("theme", pref_theme, THEME_LIGHT);
    pref.get("center", pref_center, 0);

    /* set before initializing windows */
    Fl::scheme(pref_scheme.c_str());
    m_outdir_manual = fs::current_path();

    /* init windows */
    //Fl::visible_focus(0);
    init_main_window();
    init_cmd_window();
    init_settings_window();

    /* set after initializing windows */
    m_chk_center->value(pref_center);
    m_choice_colors->value(pref_theme);
    do_colors();
}


/* d'tor */
MKVextract::~MKVextract()
{
    stop_and_delete_threads();

    /* save settings */
    Fl_Preferences pref(Fl_Preferences::USER_L, VENDOR, APPNAME);
    const char *p = Fl::scheme();
    pref.set("scheme", p ? p : "base");
    pref.set("theme", m_choice_colors->value());
    pref.set("center", m_chk_center->value());

    delete m_win_set;
    delete m_win_cmd;
    delete m_win_main;
    delete m_txtbuf;
}


void MKVextract::show(const char *file)
{
    if (m_win_main->shown()) {
        return;
    }

    if (m_chk_center->value() != 0) {
        int x = (Fl::w() - m_win_main->decorated_w()) / 2;
        int y = (Fl::h() - m_win_main->decorated_h()) / 2;
        m_win_main->position(x, y);
    }

    m_win_main->show();

    Fl::lock();

    if (file && *file) {
        start_info_thread(file);
    }
}

