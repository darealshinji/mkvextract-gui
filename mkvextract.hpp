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

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Double_Window.H>

#include <filesystem>
#include <string>
#include <vector>
#include <stdio.h>

#include "dnd.hpp"
#include "check_browser.hpp"
#include "posix_thread.hpp"
#include "rotate.hpp"


class MKVextract
{
private:

    /* entry order in browser is tracks-->attachments-->timestamps-->chapters-->tags */
    size_t m_track_count = 0;
    size_t m_attach_count = 0;
    size_t m_timestamps_entry = 0;
    size_t m_chapters_entry = 0;
    size_t m_tags_entry = 0;
    std::vector<int> m_timestampIDs;
    std::vector<std::string> m_outnames;

    std::string m_file;
    std::string m_outdir_source, m_outdir_manual;
    std::vector<std::string> m_args;


    /* multithreading */
    posix_thread *m_th_info;
    posix_thread *m_th_extract;

    static void *thread_run_mkvinfo(void *p);
    void run_mkvinfo();

    static void *thread_run_mkvextract(void *p);
    void run_mkvextract();


    /* widgets */
    Fl_Double_Window *m_win;
    Fl_Double_Window *m_cmd;
    check_browser *m_browser;
    dnd_box *m_dnd_area;
    Fl_Button *m_but_outdir;
    Fl_Button *m_but_add;
    Fl_Button *m_but_extract;
    Fl_Button *m_but_cmd;
    Fl_Box *m_progress_box;
    Fl_Box *m_outdir_field;
    Fl_Box *m_infile_label;
    Fl_Check_Button *m_use_source_path;


    /* other objects */
    Fl_Native_File_Chooser *m_fcdir;
    Fl_Native_File_Chooser *m_fcfile;
    Fl_Text_Buffer *m_txtbuf;
    rotate *m_rotate;


    /* callbacks */
    static void abort_cb(Fl_Widget *, void *p);
    static void add_cb(Fl_Widget *, void *p);
    static void browse_outdir_cb(Fl_Widget *, void *p);
    static void check_outdir_cb(Fl_Widget *, void *p);
    static void clipboard_cb(Fl_Widget *, void *p);
    static void close_cb(Fl_Widget *, void *p);
    static void close_cmd_cb(Fl_Widget *, void *p);
    static void cmd_cb(Fl_Widget *, void *p);
    static void dismiss_cb(Fl_Widget *, void *p);
    static void dnd_cb(Fl_Widget *, void *p);
    static void extract_cb(Fl_Widget *, void *p);
    static void select_all_cb(Fl_Widget *, void *p);
    static void select_none_cb(Fl_Widget *, void *p);
    static void update_browser_cb(Fl_Widget *, void *p);

    void do_abort();
    void do_add();
    void do_browse_outdir();
    void do_check_outdir();
    void do_clipboard();
    void do_close();
    //void do_close_cmd();
    void do_cmd();
    //void do_dismiss();
    void do_dnd();
    void do_extract();
    void do_select_all();
    void do_select_none();
    void do_update_browser();


    std::string create_extraction_command(bool extract);
    void restore_main_window();
    bool parsemkv(std::string &error);


    static inline std::string file_stem(const std::string &path) {
        return std::filesystem::path(path).stem().string();
    }

    static inline std::string dir_name(const std::string &path) {
        return std::filesystem::path(path).parent_path().string();
    }


public:

    MKVextract();
    ~MKVextract();

    void show(const char *file);
};


std::string quote_filename(const std::string &in);

FILE *popen_vp(char **argv, pid_t &child_pid);
FILE *popen_vp(std::vector<std::string> &argv, pid_t &child_pid);

bool xml2ogm(const char *input, const char *output);

