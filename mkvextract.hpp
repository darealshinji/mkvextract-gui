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

#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <string>
#include <vector>

class Fl_Box;
class Fl_Button;
class Fl_Check_Button;
class Fl_Double_Window;
class Fl_Native_File_Chooser;
class Fl_Text_Buffer;
class Fl_Widget;
class check_browser;
class dnd_box;
class posix_thread;
class rotate;


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

    std::string m_file, m_outdir_source, m_outdir_manual;
    std::vector<std::string> m_args;


    /* mkvextract/mkvinfo */
    void run_mkvextract();
    void run_mkvinfo();
    bool parse_mkvinfo(std::string &error);


    /* these objects must be deleted in the d'tor */
    posix_thread *m_th_info;
    posix_thread *m_th_extract;
    rotate *m_rotate;
    Fl_Double_Window *m_win;
    Fl_Double_Window *m_cmd;
    Fl_Text_Buffer *m_txtbuf;
    Fl_Native_File_Chooser *m_fcdir;
    Fl_Native_File_Chooser *m_fcfile;


    /* widgets (automatically deleted) */
    check_browser *m_browser;
    dnd_box *m_dnd_area;
    Fl_Button *m_but_outdir;
    Fl_Button *m_but_add;
    Fl_Button *m_but_extract;
    Fl_Button *m_but_abort;
    Fl_Button *m_but_cmd;
    Fl_Check_Button *m_use_source_path;
    Fl_Box *m_progress_box;
    Fl_Box *m_outdir_field;
    Fl_Box *m_infile_label;


    /* actions used by callbacks */
    void do_abort();
    void do_add();
    void do_browse_outdir();
    void do_check_outdir();
    void do_clipboard();
    void do_close();
    void do_cmd();
    void do_dnd();
    void do_extract();
    void do_select_all();
    void do_select_none();
    void do_update_browser();


    std::string create_cmd(bool extract);
    void restore_main_window();


public:

    MKVextract();
    ~MKVextract();

    void show(const char *file);
};


/* very simple auto_ptr-like class */
class auto_free
{
private:
    void *m_ptr;

public:
    auto_free(void *ptr) : m_ptr(ptr)
    {}

    ~auto_free()
    {
        //puts(__PRETTY_FUNCTION__);
        free(m_ptr);
    }
};


static inline std::string file_stem(const std::string &path)
{
    return std::filesystem::path(path).stem().string();
}


static inline std::string dir_name(const std::string &path)
{
    auto str = std::filesystem::path(path).parent_path().string();
    if (!str.ends_with('/')) { str += '/'; }
    return str;
}


std::string quote_filename(const std::string &in);
void fold_text(std::string &text);
bool command_in_path(const char *command);
bool xml2ogm(const char *input, const char *output);

