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
#include <FL/Fl_SVG_Image.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Double_Window.H>
#include <FL/filename.H>
#include <fontconfig/fontconfig.h>

#include <cstdlib>
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


namespace mkvextract
{
    static void init(int but_h);
    static void cleanup();
    int start(const char *in);
}


namespace callback
{
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


namespace thread
{
    static pthread_t extract, info;
    static bool extract_init = false;
    static bool info_init = false;
    static pid_t pid = -1;

    static void start_mkvinfo();
    static void stop();
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

    static Fl_Timeout_Handler handle = callback::rotate_timeout;
    static const float speed = 0.1; /* seconds */
}


extern "C" void *get_mkv_file_info(void *);
static std::string create_extraction_command(bool extract);

static Fl_Double_Window *win = NULL;
static Fl_Double_Window *cmdWin = NULL;

static check_browser *browser = NULL;
static dnd_box *dnd_area = NULL;
static Fl_Button *but_outdir, *but_add, *but_extract, *but_cmd;
static Fl_Box *progress_box, *outdir_field, *infile_label;
static Fl_Check_Button *check_outdir;

static bool chapters = false, same_as_source = false, extract_chapters = false;
static size_t count = 0, attach_count = 0;

static std::string file, outdir_source, outdir_manual;
static std::vector<int> timestampIDs;
static std::vector<std::string> outnames;
static std::vector<std::string> args;



static inline void lock() {
    Fl::lock();
}

static inline void unlock() {
    Fl::unlock();
    Fl::awake();
}

template<typename T>
bool find_str(const std::string &haystack, const T &needle, size_t &pos) {
    return ((pos = haystack.find(needle)) != std::string::npos);
}

//template<typename T>
//bool find_str(const std::string &haystack, const T &needle) {
//    return (haystack.find(needle) != std::string::npos);
//}

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

static void thread::start_mkvinfo()
{
    if (pthread_create(&thread::info, NULL, &get_mkv_file_info, NULL) == 0) {
        thread::info_init = true;
    }
}

extern "C" void *get_mkv_file_info(void *)
{
    std::vector<std::string> tracks, attachments, names1, names2;
    std::string error;

    lock();
    dnd_area->deactivate();
    but_add->deactivate();
    unlock();

    while (!file_is_matroska(file)) {
        lock();

        fl_message_title("Warning");

        int rv = fl_choice(
            "The selected file is not of type Matroska or WebM.\n"
            "Do you want to continue anyway?",
            "   Stop   ", "Continue", "Try again");

        unlock();

        if (rv == 0) {
            /* stop */
            lock();
            dnd_area->activate();
            but_add->activate();
            unlock();
            return NULL;
        } else if (rv == 1) {
            /* continue anyway */
            break;
        }
    }

    if (!parsemkv(file, tracks, names1, attachments, names2, timestampIDs, chapters, error)) {
        lock();

        fl_message_title("Error");
        fl_message("%s", error.c_str());
        dnd_area->activate();
        but_add->activate();

        unlock();
        return NULL;
    }

    /* save input file's dirname */
    outdir_source = fs::path(file).parent_path().string();

    if (!outdir_source.ends_with('/')) {
        outdir_source += '/';
    }

    outnames.clear();
    count = tracks.size();
    attach_count = attachments.size();

    lock();
    browser->clear();
    unlock();

    for (size_t i = 0; i < count; i++) {
        lock();
        browser->add(tracks.at(i).c_str());
        unlock();
        outnames.push_back(names1.at(i));
    }

    for (size_t i = 0; i < attach_count; i++) {
        lock();
        browser->add(attachments.at(i).c_str());
        unlock();
        outnames.push_back(names2.at(i));
    }

    lock();

    if (chapters) {
        browser->add("Chapters (xml and ogm/txt)");
    }
    browser->add("Video timestamps");
    browser->add("Tags");

    infile_label->copy_label(file.c_str());

    check_outdir->activate();
    dnd_area->activate();
    but_add->activate();
    Fl::redraw();

    unlock();

    tracks.clear();
    names1.clear();
    attachments.clear();
    names2.clear();

    return NULL;
}


static void restore_main_window()
{
    dnd_area->activate();
    check_outdir->activate();
    but_outdir->activate();
    but_add->activate();

    but_extract->label("Extract");
    but_extract->callback(callback::extract);

    Fl::remove_timeout(rotate::handle);
    rotate::box->image(NULL);

    Fl::redraw();
}

static void callback::rotate(Fl_Widget *)
{
    if (++rotate::frame == rotate::array.end()) {
        rotate::frame = rotate::array.begin();
    }

    rotate::box->image(*rotate::frame);
    rotate::box->parent()->redraw();

    Fl::repeat_timeout(rotate::speed, rotate::handle);
}

static void callback::rotate_timeout(void *)
{
    callback::rotate(NULL);
}

static void callback::dnd(Fl_Widget *)
{
    std::string items(Fl::event_text());
    std::size_t pos;

    if (find_str(items, '\n', pos)) {
        items.erase(pos);

        if (items.starts_with("file:///")) {
            /* URI */
            char *copy = strdup(items.c_str());
            fl_decode_uri(copy);
            file = copy + 7;
            free(copy);
            thread::start_mkvinfo();
        } else if (items.starts_with('/')) {
            file = items;
            thread::start_mkvinfo();
        }
    }
}

static void callback::browse_outdir(Fl_Widget *)
{
    const char *ptr;

    auto o = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    o->title("Select output directory");

    if (o->show() == 0 && (ptr = o->filename()) != NULL && *ptr != 0) {
        outdir_manual = ptr;

        if (!outdir_manual.ends_with('/')) {
            outdir_manual += '/';
        }

        if (!same_as_source) {
            outdir_field->copy_label(outdir_manual.c_str());
        }
    }

    delete o;
}

static void callback::add(Fl_Widget *, void *)
{
    const char *ptr;

    auto o = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
    o->title("Select a file");
    o->filter("*.mkv|*.mk3d|*.mka|*.mks|*.webm");

    if (o->show() == 0 && (ptr = o->filename()) != NULL && *ptr != 0) {
        file = ptr;
        thread::start_mkvinfo();
    }

    delete o;
}

extern "C" void *run_extraction_command(void *)
{
    FILE *fp;
    std::string base, xml, ogm;
    char *line = NULL;
    size_t n = 0;

    const char keyword[] = "#GUI#progress ";
    const size_t keyword_len = sizeof(keyword)-1;

    if (thread::pid > getpid()) {
        kill(thread::pid, 1);
        thread::pid = -1;
    }

    if (system("mkvextract --version 2>/dev/null >/dev/null") != 0) {
        lock();
        fl_message_title("Error");
        fl_message("%s", "mkvextract doesn't seem to be in PATH!");
        unlock();
        return NULL;
    }

    create_extraction_command(true);

    lock();

    dnd_area->deactivate();
    check_outdir->deactivate();
    but_outdir->deactivate();
    but_add->deactivate();

    but_extract->label("Abort");
    but_extract->callback(callback::abort);
    Fl::add_timeout(rotate::speed, rotate::handle);

    unlock();

    if ((fp = popen_vp(args, thread::pid)) == NULL) {
        lock();
        progress_box->label("ERROR");
        extract_chapters = false;
        unlock();
    } else {
        while (getline(&line, &n, fp) != -1) {
            if (line && strncmp(line, keyword, keyword_len) == 0) {
                lock();
                /* trailing newline is ignored by label() */
                progress_box->copy_label(line + keyword_len);
                unlock();
            }
        }

        free(line);
        thread::pid = -1;

        const char *l = (fclose(fp) == 0) ? "DONE" : "ERROR";

        lock();
        progress_box->label(l);
        unlock();
    }

    lock();
    restore_main_window();
    unlock();

    if (chapters && extract_chapters) {
        if (same_as_source) {
            base = outdir_source + fs::path(file).stem().string();
        } else {
            base = outdir_manual + fs::path(file).stem().string();
        }

        xml = base + " - chapters.xml";
        ogm = base + " - chapters.txt";

        if (!xml2ogm(xml.c_str(), ogm.c_str())) {
            lock();
            fl_message_title("Error");
            fl_message("%s", "Could not create OGM format chapters from XML!");
            unlock();
        }
    }

    return NULL;
}

static std::string create_extraction_command(bool extract)
{
    bool has_tracks = false, has_attach = false;
    std::string command, base, attach_dir;
    size_t timestamps_entry, tags_entry, chapters_entry;

    extract_chapters = false;
    args.clear();

    if (extract) {
        args.push_back("mkvextract");
        args.push_back(file);
        args.push_back("--ui-language");
        args.push_back("en_US");
        args.push_back("--gui-mode");
    } else {
        command = "mkvextract " + quote_filename(file);
    }

    if (same_as_source) {
        base = outdir_source + fs::path(file).stem().string();
    } else {
        base = outdir_manual + fs::path(file).stem().string();
    }

    /* tracks */
    for (size_t i = 0; i < count; i++) {
        if (!browser->checked(i+1)) {
            continue;
        }

        if (!has_tracks) {
            has_tracks = true;

            if (extract) {
                args.push_back("tracks");
            } else {
                command += " tracks";
            }
        }

        std::stringstream ss;
        ss << i << ":" << base << " - " << outnames.at(i);

        if (extract) {
            args.push_back(ss.str());
        } else {
            command += " " + quote_filename(ss.str());
        }
    }

    /* attachments */
    if (attach_count > 0) {
        attach_dir = base + " - Attachments/";

        for (size_t i = 0; i < attach_count; i++) {
            if (!browser->checked(i+count+1)) {
                continue;
            }

            if (!has_attach) {
                has_attach = true;

                if (extract) {
                    args.push_back("attachments");
                } else {
                    command += " attachments";
                }
            }

            std::stringstream ss;
            ss << i+1 << ":" << attach_dir << outnames.at(i+count);

            if (extract) {
                args.push_back(ss.str());
            } else {
                command += " " + quote_filename(ss.str());
            }
        }
    }

    timestamps_entry = count + attach_count + 1;
    tags_entry = timestamps_entry + 1;

    /* chapters */
    if (chapters) {
        chapters_entry = timestamps_entry;
        timestamps_entry++;
        tags_entry++;

        if (browser->checked(chapters_entry)) {
            std::string s = base + " - chapters.xml";

            if (extract) {
                args.push_back("chapters");
                args.push_back(s);
                extract_chapters = true;
            } else {
                command += " chapters " + quote_filename(s);
            }
        }
    }

    /* timestamps */
    if (browser->checked(timestamps_entry) && timestampIDs.size() > 0) {
        if (extract) {
            args.push_back("timestamps_v2");
        } else {
            command += " timestamps_v2";
        }

        for (size_t i = 0; i < timestampIDs.size(); i++) {
            int id = timestampIDs.at(i);
            std::stringstream ss;
            ss << id << ":" << base << " - track_" << id+1 << "_video_timestamps_v2.txt";

            if (extract) {
                args.push_back(ss.str());
            } else {
                command += " " + quote_filename(ss.str());
            }
        }
    }

    /* tags */
    if (browser->checked(tags_entry)) {
        std::string s = base + " - tags.xml";

        if (extract) {
            args.push_back("tags");
            args.push_back(s);
        } else {
            command += " tags " + quote_filename(s);
        }
    }

    return command;
}

static void callback::clipboard(Fl_Widget *, void *p)
{
    auto o = reinterpret_cast<Fl_Text_Buffer *>(p);
    char *text = o->text();
    Fl::copy(text, o->length(), 1);
    free(text);
}

static void callback::close_cmdWin(Fl_Widget *)
{
    if (cmdWin) {
        cmdWin->hide();
    }
}

static void callback::cmd(Fl_Widget *, void *p)
{
    auto o = reinterpret_cast<Fl_Text_Buffer *>(p);
    std::string command = create_extraction_command(false);
    o->text(command.c_str());
    cmdWin->show();
}

static void callback::extract(Fl_Widget *)
{
    if (cmdWin) {
        cmdWin->hide();
    }

    if (pthread_create(&thread::extract, NULL, &run_extraction_command, NULL) == 0) {
        thread::extract_init = true;
    }
}

static void thread::stop()
{
    if (thread::extract_init) {
        pthread_cancel(thread::extract);
        thread::extract_init = false;
    }

    if (thread::info_init) {
        pthread_cancel(thread::info);
        thread::info_init = false;
    }

    if (thread::pid > getpid()) {
        kill(thread::pid, 1);
        thread::pid = -1;
    }
}

static void callback::abort(Fl_Widget *)
{
    thread::stop();
    progress_box->label("STOPPED");
    restore_main_window();
}


static void callback::check_outdir(Fl_Widget *)
{
    if (same_as_source) {
        outdir_field->copy_label(outdir_manual.c_str());
        outdir_field->activate();
        same_as_source = false;
    } else {
        outdir_field->copy_label(outdir_source.c_str());
        outdir_field->deactivate();
        same_as_source = true;
    }
}


static void callback::update_browser(Fl_Widget *)
{
    if (browser->nchecked() > 0) {
        but_extract->activate();
        but_cmd->activate();
        progress_box->label("READY");
    } else {
        but_extract->deactivate();
        but_cmd->deactivate();
        progress_box->label(NULL);
    }
}

static void callback::close(Fl_Widget *, void *)
{
    thread::stop();

    if (cmdWin) {
        cmdWin->hide();
    }

    win->hide();
}

static void callback::select_all(Fl_Widget *, void *)
{
    browser->check_all();
    callback::update_browser(NULL);
}

static void callback::select_none(Fl_Widget *, void *)
{
    browser->check_none();
    callback::update_browser(NULL);
}

static void callback::null(Fl_Widget *, void *)
{
}

static void mkvextract::init(int but_h)
{
    /* set destination to current directory */
    char *p = get_current_dir_name();

    if (p && *p) {
        outdir_manual = p;

        if (outdir_manual.back() != '/') {
            outdir_manual += '/';
        }
    } else {
        outdir_manual = "/tmp/";
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
}

static void mkvextract::cleanup()
{
    thread::stop();

    if (win) delete win;
    if (cmdWin) delete cmdWin;

    while (!rotate::array.empty()) {
        delete rotate::array.back();
        rotate::array.pop_back();
    }
}


int mkvextract::start(const char *in)
{
    const int w = 800;
    const int h = 480;
    const int but_h = 28;
    const int but_w = 110;

    static Fl_Menu_Item context_menu[] = {
        { " Select all",     0, callback::select_all                         },
        { " Select none",    0, callback::select_none, NULL, FL_MENU_DIVIDER },
        { " Open file",      0, callback::add                                },
        { " Close program ", 0, callback::close,       NULL, FL_MENU_DIVIDER },
        { " Dismiss",        0, callback::null                               },
        { 0 }
    };

    /* init data */
    mkvextract::init(but_h);

    /* main window */

    auto position_at_center = [] (Fl_Double_Window *o) {
        o->position((Fl::w() - o->w()) / 2,
                    (Fl::h() - o->h()) / 2);
    };

    Fl_Box *dummy;
    Fl_Group *g, *g_top, *g_inside1, *g_inside2;
    const int align_center = FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP;
    const int align_left = FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP;

    auto buffer = new Fl_Text_Buffer();

    win = new Fl_Double_Window(w, h, "simple mkvextract GUI");
    win->callback(callback::close);
    {
        g = new Fl_Group(0,
                         h - but_h*2 - 25,
                         w,
                         but_h*2 + 25);
        {
            { auto o = but_extract = new Fl_Button(w - 10 - but_w,
                                                   h - 10 - but_h,
                                                   but_w,
                                                   but_h,
                                                   "Extract");
              o->callback(callback::extract);
              o->deactivate();
            } /* but_extract */

            { auto o = but_cmd = new Fl_Button(but_extract->x() - 10 - but_w,
                                               but_extract->y(),
                                               but_w,
                                               but_h,
                                               "Command");
              o->callback(callback::cmd, buffer);
              o->deactivate();
            } /* but_cmd */

            g_inside1 = new Fl_Group(0,
                                     but_extract->y(),
                                     w - 30 - 2*but_w,
                                     but_h);
            {
                { auto o = progress_box = new Fl_Box(10,
                                                     but_extract->y(),
                                                     but_w,
                                                     but_h);
                  o->align(align_center);
                  o->box(FL_THIN_DOWN_BOX);
                } /* progress_box */

                rotate::box = new Fl_Box(but_w + 15,
                                         progress_box->y(),
                                         but_h,
                                         but_h);

                { auto o = dummy = new Fl_Box(but_cmd->x() - 1,
                                              progress_box->y(),
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
                { auto o = outdir_field = new Fl_Box(10,
                                                     but_extract->y() - but_h - 5,
                                                     g_inside2->w() - 10,
                                                     but_h,
                                                     outdir_manual.c_str());
                  o->align(align_left);
                  o->box(FL_THIN_DOWN_BOX);
                } /* outdir_field */
            }
            g_inside2->resizable(outdir_field);
            g_inside2->end();

            { auto o = check_outdir = new Fl_Check_Button(but_extract->x(),
                                                          but_extract->y() - but_h - 5,
                                                          but_w,
                                                          but_h,
                                                          " Use Source");
              o->deactivate();
              o->callback(callback::check_outdir);
              o->clear_visible_focus();
            } /* check_outdir */

            { auto o = but_outdir = new Fl_Button(but_cmd->x(),
                                                  check_outdir->y(),
                                                  but_w,
                                                  but_h,
                                                  "Destination");
              o->callback(callback::browse_outdir);
            } /* but_outdir */
        }
        g->resizable(g_inside2);
        g->end();

        g_top = new Fl_Group(0, 0, w, but_h + 5);
        {
            { auto o = but_add = new Fl_Button(w - 10 - but_w,
                                               5,
                                               but_w,
                                               but_h,
                                               "Open file");
              o->callback(callback::add);
            } /* but_add */

            { auto o = infile_label = new Fl_Box(11,
                                                 5,
                                                 but_add->x() - 20,
                                                 but_h,
                                                 "(drag and drop a Matroska file)");
              o->align(align_left);
              o->box(FL_THIN_DOWN_BOX);
              o->labelsize(12);
            } /* infile_label */
        }
        g_top->resizable(infile_label);
        g_top->end();

        { auto o = browser = new check_browser(10,
                                               but_add->y() + but_add->h() + 5,
                                               w - 20,
                                               h - but_h*3 - 35);
          o->menu(context_menu);
          o->callback(callback::update_browser);
          o->clear_visible_focus();
        } /* browser */

        { auto o = dnd_area = new dnd_box(browser->x(),
                                          browser->y(),
                                          browser->w(),
                                          browser->h());
          o->callback(callback::dnd);
        } /* dnd_area */
    }
    win->end();
    win->resizable(browser);
    win->size_range(512, 384, Fl::w(), Fl::h());
    position_at_center(win);

    /* cmd window */
    Fl_Text_Display *disp;
    Fl_Button *but_copy, *but_close;
    Fl_Box *dummy2;
    Fl_Group *g_cmd;

    cmdWin = new Fl_Double_Window(640, 320, "Command line");
    {
        { auto o = disp = new Fl_Text_Display(15,
                                              15,
                                              cmdWin->w() - 30,
                                              cmdWin->h() - 30 - but_h);
          o->buffer(buffer);
          o->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 2);
        } /* disp */

        g_cmd = new Fl_Group(0,
                             disp->h() + disp->y(),
                             cmdWin->w(),
                             cmdWin->h() - disp->h() - disp->y());
        {
            { auto o = but_close = new Fl_Button(cmdWin->w() - 110 - 15,
                                                 disp->h() + disp->y() + 6,
                                                 110,
                                                 but_h,
                                                 "Close");
              o->callback(callback::close_cmdWin);
            } /* but_close */

            { auto o = but_copy = new Fl_Button(but_close->x() - 150 - 5,
                                                but_close->y(),
                                                150,
                                                but_h,
                                                "Copy to clipboard");
              o->callback(callback::clipboard, buffer);
            } /* but_copy */

            dummy2 = new Fl_Box(but_copy->x() - 1,
                                but_copy->y(),
                                1,
                                1);
        }
        g_cmd->end();
        g_cmd->resizable(dummy2);
    }
    cmdWin->resizable(disp);
    position_at_center(cmdWin);

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
                file = std::string(p);
                free(p);
            } else {
                file = std::string(in);
            }
        }
    }

    win->show();

    lock();

    if (!file.empty()) {
        thread::start_mkvinfo();
    }

    /* uncomment to test rotating animation */
    //Fl::add_timeout(rotate::speed, rotate::handle);

    /* run */
    int rv = Fl::run();

    mkvextract::cleanup();

    return rv;
}

