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

#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

class Fl_Box;
class Fl_Button;
class Fl_Check_Button;
class Fl_Choice;
class Fl_Double_Window;
struct Fl_Menu_Item;
class Fl_Native_File_Chooser;
class Fl_Text_Buffer;
class Fl_Widget;
class check_browser;
class dnd_window;
class rotate_box;


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


class MKVextract
{
private:

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

        /* create_cmd() options */
        CREATE_VECTOR,
        CREATE_TEXTBUFFER,

        /* color themes */
        THEME_LIGHT    = 0,
        THEME_DARK     = 1,

        /* Fl::copy() options */
        COPY_SELECTION = 0,
        COPY_CLIPBOARD = 1,
        COPY_BOTH      = 2
    };

    /* entry order in browser is tracks-->attachments-->timestamps-->chapters-->tags */
    size_t m_track_count = 0;
    size_t m_attach_count = 0;
    size_t m_timestamps_entry = 0;
    size_t m_chapters_entry = 0;
    size_t m_tags_entry = 0;
    std::vector<int> m_timestampIDs;
    std::vector<std::string> m_outnames;

    fs::path m_file, m_outdir_source, m_outdir_manual;
    std::vector<std::string> m_args;

    /* mkvextract/mkvinfo */
    void run_mkvextract(std::stop_token &stoken);
    void run_mkvinfo(std::stop_token stoken);
    int parse_mkvinfo(std::stop_token &stoken, std::string &error);

    /* multithreading */
    std::binary_semaphore m_extract_semaphore;
    std::jthread *m_th_extract;
    std::jthread *m_th_info;
    void start_info_thread(const char *file);
    void start_extract_thread();
    bool extraction_in_progress();
    void stop_and_delete_threads();

    /* these objects must be deleted in the d'tor */
    dnd_window *m_win_main;
    Fl_Double_Window *m_win_cmd;
    Fl_Double_Window *m_win_set;
    Fl_Text_Buffer *m_txtbuf;

    /* other widgets */
    check_browser *m_browser;
    Fl_Button *m_but_outdir;
    Fl_Button *m_but_open;
    Fl_Button *m_but_extract;
    Fl_Button *m_but_abort;
    Fl_Button *m_but_cmd;
    Fl_Check_Button *m_chk_sourcepath;
    Fl_Check_Button *m_chk_center;
    Fl_Choice *m_choice_colors;
    rotate_box *m_box_rotate;
    Fl_Box *m_box_progress;
    Fl_Box *m_box_outdir;
    Fl_Box *m_box_infile;

    /* actions used by callbacks */
    void do_abort();
    void do_check_outdir();
    void do_cmd();
    void do_colors();
    void do_copy_command();
    void do_dnd();
    void do_extract();
    void do_open_file();
    void do_quit();
    void do_select_all();
    void do_select_none();
    void do_select_attach();
    void do_set_outdir();
    void do_settings();
    void do_update_browser();

    void create_cmd(int val);

    std::string stem_path();

    void restore_main_window();
    void init_main_window();
    void init_cmd_window();
    void init_settings_window();


public:

    MKVextract();
    ~MKVextract();

    void show(const char *file);
};


namespace utils
{
    std::string quote_filename(const std::string &in);
    void fold_text(std::string &text);
    bool command_in_path(const char *command);
    bool xml2ogm(const std::string &input, const std::string &output);
}


namespace svg
{
    /* application icon */
    extern const unsigned char *mkvextract_opt;
    extern size_t mkvextract_opt_len;

    /* gear icon */
    extern const unsigned char *settings;
    extern size_t settings_len;
}

