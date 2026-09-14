/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2026 djcj <djcj@gmx.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dnd.hpp"
#include "check_browser.hpp"
#include "mkvextract.hpp"

namespace fs = std::filesystem;


struct config {
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
};


/* MKVextract */
namespace ex {
    struct config cfg;

    static void init(int but_h);
    int start(const char *in);
}


/* callback functions */
namespace cb {
    static void abort(Fl_Widget *);
    static void add(Fl_Widget *, void *);
    static void browse_outdir(Fl_Widget *);
    static void check_outdir(Fl_Widget *);
    static void clipboard(Fl_Widget *, void *);
    static void close_cmdWin(Fl_Widget *);
    static void close(Fl_Widget *, void *);
    static void cmd(Fl_Widget *, void *);
    static void dnd(Fl_Widget *);
    static void extract(Fl_Widget *);
    static void null(Fl_Widget *, void *);
    static void rotate_timeout(void *);
    static void rotate(Fl_Widget *);
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
};


/* pthread */
namespace th {
    static pthread_t extract, info;
    static bool extract_init = false;
    static bool info_init = false;
    static pid_t pid = -1;

    static void start_mkvinfo();
    static void stop();

    static inline void lock() {
        Fl::lock();
    }

    static inline void unlock() {
        Fl::unlock();
        Fl::awake();
    }
}


namespace rotate
{
    static const char svg_template[] =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<svg width=\"1024\" height=\"1024\" version=\"1.1\" viewBox=\"0 0 270.93 270.93\""
        " xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">"
            "<g transform=\"matrix(.82922 0 0 .82922 23.133 1.5165)\" stroke-width=\"0\">"
                "<rect transform=\"rotate(0)\"   x=\"0\"       y=\"152.81\"  width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
                "<rect transform=\"rotate(45)\"  x=\"74.544\"  y=\"9.7088\"  width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
                "<rect transform=\"rotate(90)\"  x=\"26.067\"  y=\"-144.19\" width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
                "<rect transform=\"rotate(135)\" x=\"-117.03\" y=\"-218.73\" width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
                "<rect transform=\"rotate(0)\"   x=\"181.1\"   y=\"152.81\"  width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
                "<rect transform=\"rotate(45)\"  x=\"255.64\"  y=\"9.7088\"  width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
                "<rect transform=\"rotate(90)\"  x=\"207.17\"  y=\"-144.19\" width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
                "<rect transform=\"rotate(135)\" x=\"64.064\"  y=\"-218.73\" width=\"89.834\" height=\"17.446\" ry=\"3.8885\" fill=\"#%s\" />"
            "</g>"
        "</svg>";

    static std::vector<Fl_SVG_Image *> array;
    static std::vector<Fl_SVG_Image *>::iterator frame;
    static Fl_Box *box = NULL;

    static Fl_Timeout_Handler handle = cb::rotate_timeout;
    static const float speed = 0.1; /* seconds */
}


extern "C" void *get_mkv_file_info(void *);
static std::string create_extraction_command(bool extract);


static inline std::string file_stem(const std::string &path) {
    return fs::path(path).stem().string();
}

static inline std::string dir_name(const std::string &path) {
    return fs::path(path).parent_path().string();
}


static std::string quote_filename(const std::string in)
{
    std::string str = "'";

    for (auto &c : in) {
        switch(c)
        {
        case '\n':
            str += "\\n";
            break;
        case '\\':
            str += "\\\\";
            break;
        case '\'':
            str += "'\\''";
            break;
        default:
            str += c;
            break;
        }
    }

    str += "'";

    return str;
}


static bool file_is_matroska(std::string &file)
{
    FILE *fp;
    unsigned char bytes[4];

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

static void th::start_mkvinfo()
{
    if (pthread_create(&th::info, NULL, &get_mkv_file_info, NULL) == 0) {
        th::info_init = true;
    }
}

extern "C" void *get_mkv_file_info(void *)
{
    std::vector<std::string> tracks, attachments, names1, names2;
    std::string error;

    th::lock();
    fltk::dnd_area->deactivate();
    fltk::but_add->deactivate();
    th::unlock();

    while (!file_is_matroska(ex::cfg.file)) {
        th::lock();

        fl_message_title("Warning");

        int rv = fl_choice(
            "The selected file is not of type Matroska or WebM.\n"
            "Do you want to continue anyway?",
            "   Stop   ", "Continue", "Try again");

        th::unlock();

        if (rv == 0) {
            /* stop */
            th::lock();
            fltk::dnd_area->activate();
            fltk::but_add->activate();
            th::unlock();

            return NULL;
        } else if (rv == 1) {
            /* continue anyway */
            break;
        }
    }

    if (!parsemkv(ex::cfg.file,
                  tracks,
                  names1,
                  attachments,
                  names2,
                  ex::cfg.timestampIDs,
                  ex::cfg.chapters,
                  error))
    {
        th::lock();

        fl_message_title("Error");
        fl_message("%s", error.c_str());
        fltk::dnd_area->activate();
        fltk::but_add->activate();

        th::unlock();

        return NULL;
    }

    /* save input file's dirname */
    ex::cfg.outdir_source = dir_name(ex::cfg.file);

    if (!ex::cfg.outdir_source.ends_with('/')) {
        ex::cfg.outdir_source += '/';
    }

    ex::cfg.outnames.clear();
    ex::cfg.track_count = tracks.size();
    ex::cfg.attach_count = attachments.size();

    th::lock();
    fltk::browser->clear();
    th::unlock();

    for (size_t i = 0; i < ex::cfg.track_count; i++) {
        th::lock();
        fltk::browser->add(tracks.at(i).c_str());
        th::unlock();

        ex::cfg.outnames.push_back(names1.at(i));
    }

    for (size_t i = 0; i < ex::cfg.attach_count; i++) {
        th::lock();
        fltk::browser->add(attachments.at(i).c_str());
        th::unlock();

        ex::cfg.outnames.push_back(names2.at(i));
    }

    th::lock();

    if (ex::cfg.chapters) {
        fltk::browser->add("Chapters (xml + ogm/txt)");
    }
    fltk::browser->add("Video timestamps");
    fltk::browser->add("Tags");

    /* activate "Select ..." menu entries */
    auto menu = fltk::browser->menu();
    menu->activate();
    menu->next()->activate();

    fltk::infile_label->copy_label(ex::cfg.file.c_str());
    fltk::use_source_path->activate();
    fltk::dnd_area->activate();
    fltk::but_add->activate();
    Fl::redraw();

    th::unlock();

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

    Fl::remove_timeout(rotate::handle);
    rotate::box->image(NULL);

    Fl::redraw();
}

static void cb::rotate(Fl_Widget *)
{
    if (++rotate::frame == rotate::array.end()) {
        rotate::frame = rotate::array.begin();
    }

    rotate::box->image(*rotate::frame);
    rotate::box->parent()->redraw();

    Fl::repeat_timeout(rotate::speed, rotate::handle);
}

static void cb::rotate_timeout(void *)
{
    cb::rotate(NULL);
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
            ex::cfg.file = copy + 7;
            free(copy);
            th::start_mkvinfo();
        } else if (items.starts_with('/')) {
            ex::cfg.file = items;
            th::start_mkvinfo();
        }
    }
}

static void cb::browse_outdir(Fl_Widget *)
{
    const char *ptr;

    Fl_Native_File_Chooser fc(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    fc.title("Select output directory");

    if (fc.show() == 0 && (ptr = fc.filename()) != NULL && *ptr != 0) {
        ex::cfg.outdir_manual = ptr;

        if (!ex::cfg.outdir_manual.ends_with('/')) {
            ex::cfg.outdir_manual += '/';
        }

        fltk::outdir_field->copy_label(ex::cfg.outdir_manual.c_str());
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
        ex::cfg.file = ptr;
        th::start_mkvinfo();
    }
}

extern "C" void *run_extraction_command(void *)
{
    FILE *fp;
    std::string base, xml, ogm;
    char *line = NULL;
    size_t n = 0;

    const char keyword[] = "#GUI#progress ";
    const size_t keyword_len = sizeof(keyword)-1;

    if (th::pid > getpid()) {
        kill(th::pid, 1);
        th::pid = -1;
    }

    if (system("mkvextract --version 2>/dev/null >/dev/null") != 0) {
        th::lock();
        fl_message_title("Error");
        fl_message("%s", "mkvextract doesn't seem to be in PATH!");
        th::unlock();
        return NULL;
    }

    create_extraction_command(true);

    th::lock();

    fltk::dnd_area->deactivate();
    fltk::use_source_path->deactivate();
    fltk::but_outdir->deactivate();
    fltk::but_add->deactivate();

    fltk::but_extract->label("Abort");
    fltk::but_extract->callback(cb::abort);
    Fl::add_timeout(rotate::speed, rotate::handle);

    th::unlock();

    if ((fp = popen_vp(ex::cfg.args, th::pid)) == NULL) {
        th::lock();
        fltk::progress_box->label("ERROR");
        ex::cfg.extract_chapters = false;
        th::unlock();
    } else {
        while (getline(&line, &n, fp) != -1) {
            if (line && strncmp(line, keyword, keyword_len) == 0) {
                th::lock();
                /* trailing newline is ignored by label() */
                fltk::progress_box->copy_label(line + keyword_len);
                th::unlock();
            }
        }

        free(line);
        th::pid = -1;

        const char *l = (fclose(fp) == 0) ? "DONE" : "ERROR";

        th::lock();
        fltk::progress_box->label(l);
        th::unlock();
    }

    th::lock();
    restore_main_window();
    th::unlock();

    if (ex::cfg.chapters && ex::cfg.extract_chapters) {
        if (fltk::use_source_path->value() == true) {
            base = ex::cfg.outdir_source + file_stem(ex::cfg.file);
        } else {
            base = ex::cfg.outdir_manual + file_stem(ex::cfg.file);
        }

        xml = base + " - chapters.xml";
        ogm = base + " - chapters.txt";

        if (!xml2ogm(xml.c_str(), ogm.c_str())) {
            th::lock();
            fl_message_title("Error");
            fl_message("%s", "Could not create OGM format chapters from XML!");
            th::unlock();
        }
    }

    return NULL;
}

static std::string create_extraction_command(bool extract)
{
    bool has_tracks = false, has_attach = false;
    std::string command, base, attach_dir;
    size_t timestamps_entry, tags_entry, chapters_entry;

    ex::cfg.extract_chapters = false;
    ex::cfg.args.clear();

    if (extract) {
        ex::cfg.args.push_back("mkvextract");
        ex::cfg.args.push_back(ex::cfg.file);
        ex::cfg.args.push_back("--ui-language");
        ex::cfg.args.push_back("en_US");
        ex::cfg.args.push_back("--gui-mode");
    } else {
        command = "mkvextract " + quote_filename(ex::cfg.file);
    }

    if (fltk::use_source_path->value() == true) {
        base = ex::cfg.outdir_source + file_stem(ex::cfg.file);
    } else {
        base = ex::cfg.outdir_manual + file_stem(ex::cfg.file);
    }

    /* tracks */
    for (size_t i = 0; i < ex::cfg.track_count; i++) {
        if (!fltk::browser->checked(i+1)) {
            continue;
        }

        if (!has_tracks) {
            has_tracks = true;

            if (extract) {
                ex::cfg.args.push_back("tracks");
            } else {
                command += " tracks";
            }
        }

        std::stringstream ss;
        ss << i << ":" << base << " - " << ex::cfg.outnames.at(i);

        if (extract) {
            ex::cfg.args.push_back(ss.str());
        } else {
            command += " " + quote_filename(ss.str());
        }
    }

    /* attachments */
    if (ex::cfg.attach_count > 0) {
        attach_dir = base + " - Attachments/";

        for (size_t i = 0; i < ex::cfg.attach_count; i++) {
            if (!fltk::browser->checked(i + ex::cfg.track_count + 1)) {
                continue;
            }

            if (!has_attach) {
                has_attach = true;

                if (extract) {
                    ex::cfg.args.push_back("attachments");
                } else {
                    command += " attachments";
                }
            }

            std::stringstream ss;
            ss << i+1 << ":" << attach_dir << ex::cfg.outnames.at(i + ex::cfg.track_count);

            if (extract) {
                ex::cfg.args.push_back(ss.str());
            } else {
                command += " " + quote_filename(ss.str());
            }
        }
    }

    timestamps_entry = ex::cfg.track_count + ex::cfg.attach_count + 1;
    tags_entry = timestamps_entry + 1;

    /* chapters */
    if (ex::cfg.chapters) {
        chapters_entry = timestamps_entry;
        timestamps_entry++;
        tags_entry++;

        if (fltk::browser->checked(chapters_entry)) {
            std::string s = base + " - chapters.xml";

            if (extract) {
                ex::cfg.args.push_back("chapters");
                ex::cfg.args.push_back(s);
                ex::cfg.extract_chapters = true;
            } else {
                command += " chapters " + quote_filename(s);
            }
        }
    }

    /* timestamps */
    if (fltk::browser->checked(timestamps_entry) && ex::cfg.timestampIDs.size() > 0) {
        if (extract) {
            ex::cfg.args.push_back("timestamps_v2");
        } else {
            command += " timestamps_v2";
        }

        for (size_t i = 0; i < ex::cfg.timestampIDs.size(); i++) {
            int id = ex::cfg.timestampIDs.at(i);
            std::stringstream ss;
            ss << id << ":" << base << " - track_" << id+1 << "_video_timestamps_v2.txt";

            if (extract) {
                ex::cfg.args.push_back(ss.str());
            } else {
                command += " " + quote_filename(ss.str());
            }
        }
    }

    /* tags */
    if (fltk::browser->checked(tags_entry)) {
        std::string s = base + " - tags.xml";

        if (extract) {
            ex::cfg.args.push_back("tags");
            ex::cfg.args.push_back(s);
        } else {
            command += " tags " + quote_filename(s);
        }
    }

    return command;
}

static void cb::clipboard(Fl_Widget *, void *p)
{
    auto buff = reinterpret_cast<Fl_Text_Buffer *>(p);
    char *text = buff->text();
    Fl::copy(text, buff->length(), 1);
    free(text);
}

static void cb::close_cmdWin(Fl_Widget *)
{
    fltk::cmdWin->hide();
}

static void cb::cmd(Fl_Widget *, void *p)
{
    std::string command = create_extraction_command(false);
    reinterpret_cast<Fl_Text_Buffer *>(p)->text(command.c_str());

    fltk::cmdWin->show();
}

static void cb::extract(Fl_Widget *)
{
    if (fltk::cmdWin) {
        fltk::cmdWin->hide();
    }

    if (pthread_create(&th::extract, NULL, &run_extraction_command, NULL) == 0) {
        th::extract_init = true;
    }
}

static void th::stop()
{
    if (th::extract_init) {
        pthread_cancel(th::extract);
        th::extract_init = false;
    }

    if (th::info_init) {
        pthread_cancel(th::info);
        th::info_init = false;
    }

    if (th::pid > getpid()) {
        kill(th::pid, 1);
        th::pid = -1;
    }
}

static void cb::abort(Fl_Widget *)
{
    th::stop();
    fltk::progress_box->label("STOPPED");
    restore_main_window();
}


static void cb::check_outdir(Fl_Widget *)
{
    if (fltk::use_source_path->value() == true) {
        fltk::outdir_field->copy_label(ex::cfg.outdir_source.c_str());
        fltk::outdir_field->deactivate();
    } else {
        fltk::outdir_field->copy_label(ex::cfg.outdir_manual.c_str());
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

static void cb::close(Fl_Widget *, void *p)
{
    th::stop();
    fltk::cmdWin->hide();
    reinterpret_cast<Fl_Double_Window *>(p)->hide();
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

static void cb::null(Fl_Widget *, void *)
{
}

static void ex::init(int but_h)
{
    /* set destination to current directory */
    char *p = get_current_dir_name();

    if (p && *p) {
        ex::cfg.outdir_manual = p;

        if (ex::cfg.outdir_manual.back() != '/') {
            ex::cfg.outdir_manual += '/';
        }
    } else {
        ex::cfg.outdir_manual = "/tmp/";
    }

    free(p);

    /* init fontconfig */
    FcInit();

    /* create rotation symbols */
    std::vector<const char *> v = { /* color values */
        "555", "999", "ddd", "000",  "000", "000", "000", "000"
    };

    size_t size = sizeof(rotate::svg_template) + 3*8;
    auto buf = new char[size];

    for (size_t i = 0; i < v.size(); i++) {
        if (i > 0) {
            /* rotate/shift color entries */
            v.insert(v.begin(), v.back());
            v.pop_back();
        }

        snprintf(buf, size, rotate::svg_template,
            v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
        rotate::array.push_back(new Fl_SVG_Image(NULL, buf));
        rotate::array.back()->resize(but_h, but_h);
    }

    rotate::frame = rotate::array.begin();

    delete[] buf;

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
        if (fs::is_regular_file(fs::status(e))) {
            Fl_PNG_Image icon(e);

            if (!icon.fail()) {
                Fl_Window::default_icon(&icon);
            }

            break;
        }
    }
}


int ex::start(const char *in)
{
    const int w = 800;
    const int h = 480;
    const int but_h = 28;
    const int but_w = 110;
    const int align_center = FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP;
    const int align_left = FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP;

    auto position_at_center = [] (Fl_Double_Window *o) {
        o->position((Fl::w() - o->decorated_w()) / 2,
                    (Fl::h() - o->decorated_h()) / 2);
    };

    /* init data */
    ex::init(but_h);

    /* main window */
    Fl_Box *dummy;
    Fl_Group *g, *g_top, *g_inside1, *g_inside2;
    auto buffer = new Fl_Text_Buffer();
    auto win = new Fl_Double_Window(w, h, "simple mkvextract GUI");

    Fl_Menu_Item context_menu[] = {
        { " Select all",     0, cb::select_all,  NULL, FL_MENU_INACTIVE                   },
        { " Select none",    0, cb::select_none, NULL, FL_MENU_INACTIVE | FL_MENU_DIVIDER },
        { " Open file",      0, cb::add                                                   },
        { " Close program ", 0, cb::close,       win,  FL_MENU_DIVIDER                    },
        { " Dismiss",        0, cb::null                                                  },
        { 0 }
    };

    win->callback(cb::close, win);
    {
        g = new Fl_Group(0,
                         h - but_h*2 - 25,
                         w,
                         but_h*2 + 25);
        {
            { auto o = fltk::but_extract =
                new Fl_Button(w - 10 - but_w,
                              h - 10 - but_h,
                              but_w,
                              but_h,
                              "Extract");
              o->callback(cb::extract);
              o->deactivate();
            } /* fltk::but_extract */

            { auto o = fltk::but_cmd =
                new Fl_Button(fltk::but_extract->x() - 10 - but_w,
                              fltk::but_extract->y(),
                              but_w,
                              but_h,
                              "Command");
              o->callback(cb::cmd, buffer);
              o->deactivate();
            } /* fltk::but_cmd */

            g_inside1 = new Fl_Group(0,
                                     fltk::but_extract->y(),
                                     w - 30 - 2*but_w,
                                     but_h);
            {
                { auto o = fltk::progress_box =
                    new Fl_Box(10,
                               fltk::but_extract->y(),
                               but_w,
                               but_h);
                  o->align(align_center);
                  o->box(FL_THIN_DOWN_BOX);
                } /* fltk::progress_box */

                rotate::box = new Fl_Box(but_w + 15,
                                         fltk::progress_box->y(),
                                         but_h,
                                         but_h);

                { auto o = dummy = new Fl_Box(fltk::but_cmd->x() - 1,
                                              fltk::progress_box->y(),
                                              1,
                                              1);
                  o->box(FL_NO_BOX);
                } /* dummy */
            }
            g_inside1->resizable(dummy);
            g_inside1->end();

            g_inside2 = new Fl_Group(0,
                                     g->y(),
                                     g_inside1->w(),
                                     but_h);
            {
                { auto o = fltk::outdir_field =
                    new Fl_Box(10,
                               fltk::but_extract->y() - but_h - 5,
                               g_inside2->w() - 10,
                               but_h,
                               ex::cfg.outdir_manual.c_str());
                  o->align(align_left);
                  o->box(FL_THIN_DOWN_BOX);
                } /* fltk::outdir_field */
            }
            g_inside2->resizable(fltk::outdir_field);
            g_inside2->end();

            { auto o = fltk::use_source_path =
                new Fl_Check_Button(fltk::but_extract->x(),
                                    fltk::but_extract->y() - but_h - 5,
                                    but_w,
                                    but_h,
                                    " Source path");
              o->deactivate();
              o->callback(cb::check_outdir);
              o->clear_visible_focus();
            } /* fltk::use_source_path */

            { auto o = fltk::but_outdir =
                new Fl_Button(fltk::but_cmd->x(),
                              fltk::use_source_path->y(),
                              but_w,
                              but_h,
                              "Destination");
              o->callback(cb::browse_outdir);
            } /* fltk::but_outdir */
        }
        g->resizable(g_inside2);
        g->end();

        g_top = new Fl_Group(0, 0, w, but_h + 5);
        {
            { auto o = fltk::but_add =
                new Fl_Button(w - 10 - but_w,
                              5,
                              but_w,
                              but_h,
                              "Open file");
              o->callback(cb::add);
            } /* fltk::but_add */

            { auto o = fltk::infile_label =
                new Fl_Box(11,
                           5,
                           fltk::but_add->x() - 20,
                           but_h,
                           "(drag and drop a Matroska file)");
              o->align(align_left);
              o->box(FL_THIN_DOWN_BOX);
              o->labelsize(12);
            } /* fltk::infile_label */
        }
        g_top->resizable(fltk::infile_label);
        g_top->end();

        { auto o = fltk::browser =
                new check_browser(10,
                                  fltk::but_add->y() + fltk::but_add->h() + 5,
                                  w - 20,
                                  h - but_h*3 - 35);
          o->menu(context_menu);
          o->callback(cb::update_browser);
          o->clear_visible_focus();
        } /* fltk::browser */

        { auto o = fltk::dnd_area = new dnd_box(fltk::browser->x(),
                                                fltk::browser->y(),
                                                fltk::browser->w(),
                                                fltk::browser->h());
          o->callback(cb::dnd);
        } /* fltk::dnd_area */
    }
    win->end();
    win->resizable(fltk::browser);
    win->size_range(512, 384, Fl::w(), Fl::h());
    position_at_center(win);

    /* cmd window */
    Fl_Text_Display *disp;
    Fl_Button *but_copy, *but_close;
    Fl_Box *dummy2;
    Fl_Group *g_cmd;

    fltk::cmdWin = new Fl_Double_Window(640, 320, "Command line");
    {
        { auto o = disp = new Fl_Text_Display(15,
                                              15,
                                              fltk::cmdWin->w() - 30,
                                              fltk::cmdWin->h() - 30 - but_h);
          o->buffer(buffer);
          o->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 2);
        } /* disp */

        g_cmd = new Fl_Group(0,
                             disp->h() + disp->y(),
                             fltk::cmdWin->w(),
                             fltk::cmdWin->h() - disp->h() - disp->y());
        {
            { auto o = but_close = new Fl_Button(fltk::cmdWin->w() - 110 - 15,
                                                 disp->h() + disp->y() + 6,
                                                 110,
                                                 but_h,
                                                 "Close");
              o->callback(cb::close_cmdWin);
            } /* but_close */

            { auto o = but_copy = new Fl_Button(but_close->x() - 150 - 5,
                                                but_close->y(),
                                                150,
                                                but_h,
                                                "Copy to clipboard");
              o->callback(cb::clipboard, buffer);
            } /* but_copy */

            dummy2 = new Fl_Box(but_copy->x() - 1,
                                but_copy->y(),
                                1,
                                1);
        }
        g_cmd->end();
        g_cmd->resizable(dummy2);
    }
    fltk::cmdWin->resizable(disp);
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
                ex::cfg.file = p;
                free(p);
            } else {
                ex::cfg.file = in;
            }
        }
    }

    win->show();

    th::lock();

    if (!ex::cfg.file.empty()) {
        th::start_mkvinfo();
    }

    /* uncomment to test rotating animation */
    //Fl::add_timeout(rotate::speed, rotate::handle);

    /* run */
    int rv = Fl::run();

    /* cleanup */
    th::stop();

    if (win) delete win;
    if (fltk::cmdWin) delete fltk::cmdWin;

    while (!rotate::array.empty()) {
        delete rotate::array.back();
        rotate::array.pop_back();
    }

    return rv;
}

