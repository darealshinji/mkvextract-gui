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
#include <FL/Fl_File_Chooser.H>
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

namespace fs = std::filesystem;


/* MKVextract */
namespace ex {
    namespace cfg {
        bool chapters;
        bool extract_chapters;
        size_t track_count;
        size_t attach_count;
        std::string file;
        std::string outdir_source;
        std::string outdir_manual;
        std::vector<int> timestampIDs;
        std::vector<std::string> outnames;
        std::vector<std::string> args;
    }

    static void init(int bt_h);
    int start(const char *in);
}


#define CB(fn) reinterpret_cast<void (*)(Fl_Widget*, void*)>(cb::fn)


/* callback functions */
namespace cb {
    static void abort(Fl_Widget *);
    static void add(Fl_Widget *, void *);
    static void browse_outdir(Fl_Widget *);
    static void check_outdir(Fl_Widget *);
    static void clipboard(Fl_Widget *, Fl_Text_Buffer *);
    static void close(Fl_Widget *, Fl_Double_Window *);
    static void cmd(Fl_Widget *, Fl_Text_Buffer *);
    static void dnd(Fl_Widget *);
    static void extract(Fl_Widget *);
    static void select_all(Fl_Widget *, void *);
    static void select_none(Fl_Widget *, void *);
    static void update_browser(Fl_Widget *);
}


namespace fltk {
    static Fl_Double_Window *cmdWin = NULL;
    static check_browser *browser = NULL;
    static dnd_box *dnd_area = NULL;
    static Fl_Button *but_outdir = NULL;
    static Fl_Button *but_add = NULL;
    static Fl_Button *but_extract = NULL;
    static Fl_Button *but_cmd = NULL;
    static Fl_Box *progress_box = NULL;
    static Fl_Box *outdir_field = NULL;
    static Fl_Box *infile_label = NULL;
    static Fl_Check_Button *use_source_path = NULL;
    static rotate *spin = NULL;
}


namespace thread {
    static void *get_mkv_file_info(void *);
    static void *run_extraction_command(void *);

    static posix_thread info(get_mkv_file_info);
    static posix_thread extract(run_extraction_command);

    static inline void lock() {
        Fl::lock();
    }

    static inline void unlock() {
        Fl::unlock();
        Fl::awake();
    }
}


static std::string create_extraction_command(bool extract);


static inline std::string file_stem(const std::string &path) {
    return fs::path(path).stem().string();
}

static inline std::string dir_name(const std::string &path) {
    return fs::path(path).parent_path().string();
}


static bool file_is_matroska(std::string &file)
{
    FILE *fp;
    uint8_t bytes[4];

    if (file.empty() || !(fp = fopen(file.c_str(), "r"))) {
        return false;
    }

    if (fread(bytes, 1, sizeof(bytes), fp) < sizeof(bytes)) {
        fclose(fp);
        return false;
    }

    fclose(fp);

    return (memcmp(bytes, "\x1A\x45\xDF\xA3", 4) == 0);
}


static void *thread::get_mkv_file_info(void *)
{
    struct mkv_file_info info;
    std::vector<std::string> tracks, attachments, names1, names2;
    std::string error;

    thread::lock();
    fltk::dnd_area->deactivate();
    fltk::but_add->deactivate();
    thread::unlock();

    while (!file_is_matroska(ex::cfg::file)) {
        thread::lock();

        fl_message_title("Warning");

        int rv = fl_choice(
            "The selected file is not of type Matroska or WebM.\n"
            "Do you want to continue anyway?",
            "   Stop   ", "Continue", "Try again");

        thread::unlock();

        if (rv == 0) {
            /* stop */
            thread::lock();
            fltk::dnd_area->activate();
            fltk::but_add->activate();
            thread::unlock();

            return NULL;
        } else if (rv == 1) {
            /* continue anyway */
            break;
        }
    }

    if (!parsemkv(ex::cfg::file, info, error)) {
        thread::lock();

        fl_message_title("Error");
        fl_message("%s", error.c_str());
        fltk::dnd_area->activate();
        fltk::but_add->activate();

        thread::unlock();

        return NULL;
    }

    /* save input file's dirname */
    ex::cfg::outdir_source = dir_name(ex::cfg::file);

    if (!ex::cfg::outdir_source.ends_with('/')) {
        ex::cfg::outdir_source += '/';
    }

    ex::cfg::outnames.clear();
    ex::cfg::track_count = info.tracks.size();
    ex::cfg::attach_count = info.attachments.size();
    ex::cfg::timestampIDs = info.timestampIDs;
    ex::cfg::chapters = info.has_chapters;

    thread::lock();
    fltk::browser->clear();
    thread::unlock();

    for (size_t i = 0; i < ex::cfg::track_count; i++) {
        thread::lock();
        fltk::browser->add(info.tracks.at(i).info.c_str());
        thread::unlock();

        ex::cfg::outnames.push_back(info.tracks.at(i).filename);
    }

    for (size_t i = 0; i < ex::cfg::attach_count; i++) {
        thread::lock();
        fltk::browser->add(info.attachments.at(i).info.c_str());
        thread::unlock();

        ex::cfg::outnames.push_back(info.attachments.at(i).filename);
    }

    thread::lock();

    if (ex::cfg::chapters) {
        fltk::browser->add("Chapters (xml + ogm/txt)");
    }
    fltk::browser->add("Video timestamps");
    fltk::browser->add("Tags");

    /* activate "Select ..." menu entries */
    auto menu = fltk::browser->menu();
    menu->activate();
    menu->next()->activate();

    /* update widgets */
    fltk::infile_label->copy_label(ex::cfg::file.c_str());
    fltk::use_source_path->activate();
    fltk::dnd_area->activate();
    fltk::but_add->activate();
    fltk::but_extract->deactivate();
    fltk::but_cmd->deactivate();
    fltk::progress_box->label(NULL);
    cb::check_outdir(NULL);

    Fl::redraw();

    thread::unlock();

    return NULL;
}


static void restore_main_window()
{
    fltk::dnd_area->activate();
    fltk::use_source_path->activate();
    fltk::but_outdir->activate();
    fltk::but_add->activate();

    fltk::but_extract->label("Extract");
    fltk::but_extract->callback(cb::extract);

    fltk::spin->deactivate();
    Fl::redraw();
}

static void cb::dnd(Fl_Widget *)
{
    std::string items(Fl::event_text());
    size_t pos = items.find('\n');

    if (pos != std::string::npos) {
        items.erase(pos);

        if (items.starts_with("file:///")) {
            /* URI */
            char *copy = strdup(items.c_str());
            fl_decode_uri(copy);
            ex::cfg::file = copy + 7;
            free(copy);
            thread::info.start();
        } else if (items.starts_with('/')) {
            ex::cfg::file = items;
            thread::info.start();
        }
    }
}

static void cb::browse_outdir(Fl_Widget *)
{
    const char *ptr;

    Fl_Native_File_Chooser fc(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    fc.title("Select output directory");

    if (fc.show() == 0 && (ptr = fc.filename()) != NULL && *ptr != 0) {
        ex::cfg::outdir_manual = ptr;

        if (!ex::cfg::outdir_manual.ends_with('/')) {
            ex::cfg::outdir_manual += '/';
        }

        fltk::outdir_field->copy_label(ex::cfg::outdir_manual.c_str());
        fltk::outdir_field->activate();
        fltk::use_source_path->clear();
    }
}

static void cb::add(Fl_Widget *, void *)
{
    const char *ptr;

    Fl_Native_File_Chooser fc(Fl_Native_File_Chooser::BROWSE_FILE);
    fc.title("Select a file");
    fc.filter("*.mkv|*.mk3d|*.mka|*.mks|*.webm");

    if (fc.show() == 0 && (ptr = fc.filename()) != NULL && *ptr != 0) {
        ex::cfg::file = ptr;
        thread::info.start();
    }
}

static void *thread::run_extraction_command(void *)
{
    std::string base, xml, ogm;
    char *line = NULL;
    size_t n = 0;

    const char keyword[] = "#GUI#progress ";
    const size_t keyword_len = sizeof(keyword)-1;

    if (system("mkvextract --version 2>/dev/null >/dev/null") != 0) {
        thread::lock();
        fl_message_title("Error");
        fl_message("%s", "mkvextract doesn't seem to be in PATH!");
        thread::unlock();
        return NULL;
    }

    create_extraction_command(true);

    thread::lock();

    fltk::dnd_area->deactivate();
    fltk::use_source_path->deactivate();
    fltk::but_outdir->deactivate();
    fltk::but_add->deactivate();

    fltk::but_extract->label("Abort");
    fltk::but_extract->callback(cb::abort);
    fltk::spin->activate();

    thread::unlock();

    pipe_command cmd(ex::cfg::args);
    FILE *fp = cmd.pipe_open();

    if (!fp) {
        thread::lock();
        fltk::progress_box->label("ERROR");
        ex::cfg::extract_chapters = false;
        thread::unlock();
    } else {
        while (getline(&line, &n, fp) != -1) {
            if (line && strncmp(line, keyword, keyword_len) == 0) {
                thread::lock();
                /* trailing newline is ignored by label() */
                fltk::progress_box->copy_label(line + keyword_len);
                thread::unlock();
            }
        }

        const char *l = cmd.error() ? "ERROR" : "DONE";
        cmd.pipe_close();
        free(line);

        thread::lock();
        fltk::progress_box->label(l);
        thread::unlock();
    }

    thread::lock();
    restore_main_window();
    thread::unlock();

    if (ex::cfg::chapters && ex::cfg::extract_chapters) {
        if (fltk::use_source_path->value() == true) {
            base = ex::cfg::outdir_source + file_stem(ex::cfg::file);
        } else {
            base = ex::cfg::outdir_manual + file_stem(ex::cfg::file);
        }

        xml = base + " - chapters.xml";
        ogm = base + " - chapters.txt";

        if (!xml2ogm(xml.c_str(), ogm.c_str())) {
            thread::lock();
            fl_message_title("Error");
            fl_message("%s", "Could not create OGM format chapters from XML!");
            thread::unlock();
        }
    }

    return NULL;
}

static std::string create_extraction_command(bool extract)
{
    bool has_tracks = false, has_attach = false;
    std::string command, base, attach_dir;
    size_t timestamps_entry, tags_entry, chapters_entry;

    ex::cfg::extract_chapters = false;
    ex::cfg::args.clear();

    if (extract) {
        ex::cfg::args.push_back("mkvextract");
        ex::cfg::args.push_back(ex::cfg::file);
        ex::cfg::args.push_back("--ui-language");
        ex::cfg::args.push_back("en_US");
        ex::cfg::args.push_back("--gui-mode");
    } else {
        command = "mkvextract " + quote_filename(ex::cfg::file);
    }

    if (fltk::use_source_path->value() == true) {
        base = ex::cfg::outdir_source + file_stem(ex::cfg::file);
    } else {
        base = ex::cfg::outdir_manual + file_stem(ex::cfg::file);
    }

    /* tracks */
    for (size_t i = 0; i < ex::cfg::track_count; i++) {
        if (!fltk::browser->checked(i+1)) {
            continue;
        }

        if (!has_tracks) {
            has_tracks = true;

            if (extract) {
                ex::cfg::args.push_back("tracks");
            } else {
                command += " tracks";
            }
        }

        std::stringstream ss;
        ss << i << ":" << base << " - " << ex::cfg::outnames.at(i);

        if (extract) {
            ex::cfg::args.push_back(ss.str());
        } else {
            command += " " + quote_filename(ss.str());
        }
    }

    /* attachments */
    if (ex::cfg::attach_count > 0) {
        attach_dir = base + " - Attachments/";

        for (size_t i = 0; i < ex::cfg::attach_count; i++) {
            if (!fltk::browser->checked(i + ex::cfg::track_count + 1)) {
                continue;
            }

            if (!has_attach) {
                has_attach = true;

                if (extract) {
                    ex::cfg::args.push_back("attachments");
                } else {
                    command += " attachments";
                }
            }

            std::stringstream ss;
            ss << i+1 << ":" << attach_dir << ex::cfg::outnames.at(i + ex::cfg::track_count);

            if (extract) {
                ex::cfg::args.push_back(ss.str());
            } else {
                command += " " + quote_filename(ss.str());
            }
        }
    }

    timestamps_entry = ex::cfg::track_count + ex::cfg::attach_count + 1;
    tags_entry = timestamps_entry + 1;

    /* chapters */
    if (ex::cfg::chapters) {
        chapters_entry = timestamps_entry;
        timestamps_entry++;
        tags_entry++;

        if (fltk::browser->checked(chapters_entry)) {
            std::string s = base + " - chapters.xml";

            if (extract) {
                ex::cfg::args.push_back("chapters");
                ex::cfg::args.push_back(s);
                ex::cfg::extract_chapters = true;
            } else {
                command += " chapters " + quote_filename(s);
            }
        }
    }

    /* timestamps */
    if (fltk::browser->checked(timestamps_entry) && ex::cfg::timestampIDs.size() > 0) {
        if (extract) {
            ex::cfg::args.push_back("timestamps_v2");
        } else {
            command += " timestamps_v2";
        }

        for (size_t i = 0; i < ex::cfg::timestampIDs.size(); i++) {
            int id = ex::cfg::timestampIDs.at(i);
            std::stringstream ss;
            ss << id << ":" << base << " - track_" << id+1 << "_video_timestamps_v2.txt";

            if (extract) {
                ex::cfg::args.push_back(ss.str());
            } else {
                command += " " + quote_filename(ss.str());
            }
        }
    }

    /* tags */
    if (fltk::browser->checked(tags_entry)) {
        std::string s = base + " - tags.xml";

        if (extract) {
            ex::cfg::args.push_back("tags");
            ex::cfg::args.push_back(s);
        } else {
            command += " tags " + quote_filename(s);
        }
    }

    return command;
}

static void cb::clipboard(Fl_Widget *, Fl_Text_Buffer *buffer)
{
    char *text = buffer->text();
    Fl::copy(text, buffer->length(), 1);
    free(text);
}

static void cb::cmd(Fl_Widget *, Fl_Text_Buffer *buffer)
{
    std::string command = create_extraction_command(false);
    buffer->text(command.c_str());
    fltk::cmdWin->show();
}

static void cb::extract(Fl_Widget *)
{
    fltk::cmdWin->hide();
    thread::extract.start();
}

static void cb::abort(Fl_Widget *)
{
    thread::extract.cancel();
    thread::info.cancel();
    fltk::progress_box->label("STOPPED");
    restore_main_window();
}


static void cb::check_outdir(Fl_Widget *)
{
    if (fltk::use_source_path->value() == true) {
        fltk::outdir_field->copy_label(ex::cfg::outdir_source.c_str());
        fltk::outdir_field->deactivate();
    } else {
        fltk::outdir_field->copy_label(ex::cfg::outdir_manual.c_str());
        fltk::outdir_field->activate();
    }
}


static void cb::update_browser(Fl_Widget *)
{
    if (fltk::browser->nchecked() > 0) {
        fltk::but_extract->activate();
        fltk::but_cmd->activate();
        fltk::progress_box->label("READY");
    } else {
        fltk::but_extract->deactivate();
        fltk::but_cmd->deactivate();
        fltk::progress_box->label(NULL);
    }
}

static void cb::close(Fl_Widget *, Fl_Double_Window *win)
{
    thread::extract.cancel();
    thread::info.cancel();
    fltk::cmdWin->hide();
    win->hide();
}

static void cb::select_all(Fl_Widget *, void *)
{
    fltk::browser->check_all();
    cb::update_browser(NULL);
}

static void cb::select_none(Fl_Widget *, void *)
{
    fltk::browser->check_none();
    cb::update_browser(NULL);
}

static void ex::init(int bt_h)
{
    /* set destination to current directory */
    char *p = get_current_dir_name();

    if (p && *p) {
        ex::cfg::outdir_manual = p;

        if (ex::cfg::outdir_manual.back() != '/') {
            ex::cfg::outdir_manual += '/';
        }
    } else {
        ex::cfg::outdir_manual = "/tmp/";
    }

    free(p);

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
}


int ex::start(const char *in)
{
    const int bt_h = 28;
    const int bt_w = 110;
    const int center_align = FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP;
    const int left_align = FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP;
    int x, y, w, h;

    auto position_at_center = [] (Fl_Double_Window *o) {
        o->position((Fl::w() - o->decorated_w()) / 2,
                    (Fl::h() - o->decorated_h()) / 2);
    };

    /* init data */
    ex::init(bt_h);

    /* main window */
    Fl_Text_Buffer buffer;
    Fl_Double_Window win1(800, 480, "simple mkvextract GUI");
    auto win = &win1;

    Fl_Menu_Item context_menu[] = {
        { " Select all",     0, cb::select_all,  NULL,    FL_MENU_INACTIVE                   },
        { " Select none",    0, cb::select_none, NULL,    FL_MENU_INACTIVE | FL_MENU_DIVIDER },
        { " Open file",      0, cb::add                                                      },
        { " Close program ", 0, CB(close),       win,     FL_MENU_DIVIDER                    },
        { " Dismiss",        0, [](Fl_Widget *, void *){}                                    },
        { 0 }
    };

    win->callback(CB(close), win);
    win->begin();

        /* upper area group */
        h = bt_h + 5;
        Fl_Group g_up(0, 0, win->w(), h);
        g_up.begin();

            /* "Open file" button */
            x = win->w() - 10 - bt_w;
            Fl_Button bt5(x, 5, bt_w, bt_h, "Open file");
            bt5.callback(cb::add);
            fltk::but_add = &bt5;

            /* input file label */
            w = fltk::but_add->x() - 20;
            Fl_Box bx4(FL_THIN_DOWN_BOX, 11, 5, w, bt_h, "(drag and drop a Matroska file)");
            bx4.align(left_align);
            bx4.labelsize(12);
            fltk::infile_label = &bx4;

        g_up.end();
        g_up.resizable(fltk::infile_label);

        /* bottom area group */
        y = win->h() - bt_h*2 - 25;
        w = win->w();
        h = bt_h*2 + 25;
        Fl_Group g_bttm(0, y, w, h);
        g_bttm.begin();

            /* "Extract" button */
            x = win->w() - 10 - bt_w;
            y = win->h() - 10 - bt_h;
            Fl_Button bt1(x, y, bt_w, bt_h, "Extract");
            bt1.callback(cb::extract);
            bt1.deactivate();
            fltk::but_extract = &bt1;

            /* "Command" button */
            x = fltk::but_extract->x() - 10 - bt_w;
            y = fltk::but_extract->y();
            Fl_Button bt2(x, y, bt_w, bt_h, "Command");
            bt2.callback(CB(cmd), &buffer);
            bt2.deactivate();
            fltk::but_cmd = &bt2;

            /* progress area group */
            y = fltk::but_extract->y();
            w = win->w() - 30 - 2*bt_w;
            Fl_Group g_prog(0, y, w, bt_h);
            g_prog.begin();

                /* progress box */
                y = fltk::but_extract->y();
                Fl_Box bx1(FL_THIN_DOWN_BOX, 10, y, bt_w, bt_h, NULL);
                bx1.align(center_align);
                fltk::progress_box = &bx1;

                /* spin/rotate icon box */
                x = bt_w + 15;
                y = fltk::progress_box->y();
                Fl_Box bx2(x, y, bt_h, bt_h);
                rotate bx2_rotate(&bx2, bt_h);
                fltk::spin = &bx2_rotate;

                /* dummy */
                x = fltk::but_cmd->x() - 1;
                y = fltk::progress_box->y();
                Fl_Box dummy1(FL_NO_BOX, x, y, 1, 1, NULL);

            g_prog.end();
            g_prog.resizable(&dummy1);

            /* output directory group */
            Fl_Group g_outd(0, g_bttm.y(), g_prog.w(), bt_h);
            g_outd.begin();

                /* output directory label */
                y = fltk::but_extract->y() - bt_h - 5;
                w = g_outd.w() - 10;
                Fl_Box bx3(FL_THIN_DOWN_BOX, 10, y, w, bt_h, ex::cfg::outdir_manual.c_str());
                bx3.align(left_align);
                fltk::outdir_field = &bx3;

            g_outd.end();
            g_outd.resizable(fltk::outdir_field);

            /* "Source path" check button */
            x = fltk::but_extract->x();
            y = fltk::but_extract->y() - bt_h - 5;
            Fl_Check_Button bt3(x, y, bt_w, bt_h, " Source path");
            bt3.deactivate();
            bt3.callback(cb::check_outdir);
            //bt3.clear_visible_focus();
            fltk::use_source_path = &bt3;

            /* "Destination" button */
            x = fltk::but_cmd->x();
            y = fltk::use_source_path->y();
            Fl_Button bt4(x, y, bt_w, bt_h, "Destination");
            bt4.callback(cb::browse_outdir);
            fltk::but_outdir = &bt4;

        g_bttm.end();
        g_bttm.resizable(g_outd);

        /* check browser */
        y = fltk::but_add->y() + fltk::but_add->h() + 5;
        w = win->w() - 20;
        h = win->h() - bt_h*3 - 35;
        check_browser chk(10, y, w, h);
        chk.menu(context_menu);
        chk.callback(cb::update_browser);
        //chk.clear_visible_focus();
        fltk::browser = &chk;

        /* drag 'n drop area */
        dnd_box bx5(win->x(), win->y(), win->w(), win->h());
        bx5.callback(cb::dnd);
        fltk::dnd_area = &bx5;

    win->end();
    win->resizable(fltk::browser);
    win->size_range(512, 384, Fl::w(), Fl::h());
    position_at_center(win);


    /* Command line window */
    Fl_Double_Window win2(640, 320, "Command line");
    fltk::cmdWin = &win2;
    fltk::cmdWin->begin();

        /* text display */
        w = fltk::cmdWin->w() - 30;
        h = fltk::cmdWin->h() - 30 - bt_h;
        Fl_Text_Display dsp(15, 15, w, h);
        dsp.buffer(&buffer);
        dsp.wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 2);

        /* button group */
        y = dsp.h() + dsp.y();
        w = fltk::cmdWin->w();
        h = fltk::cmdWin->h() - dsp.h() - dsp.y();
        Fl_Group g_bttn(0, y, w, h);
        g_bttn.begin();

            /* "Close" button */
            x = fltk::cmdWin->w() - 110 - 15;
            y = dsp.h() + dsp.y() + 6;
            Fl_Button bt6(x, y, 110, bt_h, "Close");
            auto fn_close = [] (Fl_Widget *) { fltk::cmdWin->hide(); };
            bt6.callback(fn_close);

            /* "Copy to clipboard" button */
            x = bt6.x() - 150 - 5;
            Fl_Button bt7(x, bt6.y(), 150, bt_h, "Copy to clipboard");
            bt7.callback(CB(clipboard), &buffer);

            /* dummy */
            x = bt7.x() - 1;
            Fl_Box dummy2(FL_NO_BOX, x, bt7.y(), 1, 1, NULL);

        g_bttn.end();
        g_bttn.resizable(&dummy2);

    fltk::cmdWin->end();
    fltk::cmdWin->resizable(&dsp);
    fltk::cmdWin->size_range(fltk::cmdWin->w(), fltk::cmdWin->h(), Fl::w(), Fl::h());
    position_at_center(fltk::cmdWin);


    if (in && *in) {
        if (fl_filename_isdir(in)) {
            fl_message_title("Error");
            fl_message("`%s' is a directory!", in);
        } else if (access(in, R_OK) != 0) {
            fl_message_title("Error");
            fl_message("cannot read file `%s'", in);
        } else {
            char *p;

            if (in[0] != '/' && (p = canonicalize_file_name(in)) != NULL) {
                ex::cfg::file = p;
                free(p);
            } else {
                ex::cfg::file = in;
            }
        }
    }

    win->show();
    //fltk::spin->activate(); /* test */

    thread::lock();

    if (!ex::cfg::file.empty()) {
        thread::info.start();
    }

    /* run */
    int rv = Fl::run();

    /* cleanup */
    thread::extract.cancel();
    thread::info.cancel();

    return rv;
}

