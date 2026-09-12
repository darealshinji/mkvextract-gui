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


static const char svg_rotation_template[] =
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


static const float img_duration = 0.1;  /* seconds */
static const int last_frame = 7;
static int current_frame = 0;

static std::vector<Fl_SVG_Image *> img_rotate;

static Fl_Double_Window *win = NULL;
static const int but_h = 28;

static Fl_Double_Window *cmdWin = NULL;
static Fl_Text_Buffer *cmdBuff;

static Fl_Native_File_Chooser *fc = NULL;
static check_browser *browser;
static dnd_box *dnd_area;
static Fl_Button *but_outdir, *but_add, *but_extract, *but_cmd;
static Fl_Box *progress_box, *outdir_field, *infile_label, *rotate_img;
static Fl_Check_Button *check_outdir;

static Fl_Timeout_Handler th;

static pthread_t tcom, tinfo;
static bool tcom_init = false;
static bool tinfo_init = false;
static pid_t child_pid = -1;

static bool chapters = false, same_as_source = false, extract_chapters = false;
static size_t count = 0, attach_count = 0;

static std::string file, outdir_auto, outdir_manual;
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

static FILE *popen_mkvextract()
{
    enum { r = 0, w = 1 };
    int fd[2];

    if (pipe(fd) == -1) {
        return NULL;
    }

    if ((child_pid = fork()) != 0) {
        close(fd[w]);
        return fdopen(fd[r], "r");
    }

    size_t len = args.size();
    auto child_argv = new char *[len + 1];

    for (size_t i = 0; i < len; i++) {
        child_argv[i] = const_cast<char *>(args.at(i).c_str());
    }
    child_argv[len] = NULL;

    close(fd[r]);
    dup2(fd[w], 1);
    close(fd[w]);
    execvp("mkvextract", child_argv);

    delete[] child_argv;
    _exit(127);
}


template<typename T>
bool find_str(const std::string &haystack, const T &needle, size_t &pos)
{
    return ((pos = haystack.find(needle)) != std::string::npos);
}

template<typename T>
bool find_str(const std::string &haystack, const T &needle)
{
    return (haystack.find(needle) != std::string::npos);
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


extern "C" void *get_mkv_file_info(void *);

static void mkv_file_info_create_thread()
{
    if (!file.empty() && pthread_create(&tinfo, NULL, &get_mkv_file_info, NULL) == 0) {
        tinfo_init = true;
    }
}

extern "C" void *get_mkv_file_info(void *)
{
    std::vector<std::string> tracks, attachments, names1, names2;
    std::string error;
    std::size_t pos;

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

    /* get dirname */
    outdir_auto = file;

    if (find_str(outdir_auto, '/', pos)) {
        outdir_auto = "";
    } else {
        outdir_auto.erase(pos + 1);
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
    win->redraw();

    unlock();

    tracks.clear();
    names1.clear();
    attachments.clear();
    names2.clear();

    return NULL;
}

static void rotate_callback(Fl_Widget *)
{
    if (current_frame < last_frame) {
        current_frame++;
    } else {
        current_frame = 0;
    }

    rotate_img->image(img_rotate[current_frame]);
    rotate_img->parent()->redraw();

    Fl::repeat_timeout(img_duration, th);
}

static void rotate_th_callback(void *)
{
    rotate_callback(NULL);
}

static void dnd_callback(Fl_Widget *)
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
            mkv_file_info_create_thread();
        } else if (items.starts_with('/')) {
            file = items;
            mkv_file_info_create_thread();
        }
    }
}

static void browse_outdir_cb(Fl_Widget *)
{
    if (fc) delete fc;

    fc = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    fc->title("Select output directory");

    if (fc->show() != 0) {
        return;
    }

    const char *dir = fc->filename();

    if (!dir) {
        return;
    }

    outdir_manual = std::string(dir) + "/";

    if (!same_as_source) {
        outdir_field->copy_label(outdir_manual.c_str());
    }
}

static void add_callback(Fl_Widget *, void *)
{
    if (fc) delete fc;

    fc = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
    fc->title("Select a file");
    fc->filter("*.mkv|*.mk3d|*.mka|*.mks|*.webm");

    if (fc->show() == 0 && fc->filename() != NULL) {
        file = std::string(fc->filename());
        mkv_file_info_create_thread();
    }
}

static std::string create_extraction_command(bool extract);
static void abort_cb(Fl_Widget *);
static void extract_cb(Fl_Widget *);

extern "C" void *run_extraction_command(void *)
{
    FILE *fp;
    std::string base, xml, ogm;
    char *line = NULL;
    size_t n = 0;

    const char keyword[] = "#GUI#progress ";
    const size_t keyword_len = sizeof(keyword)-1;

    if (child_pid > getpid()) {
        kill(child_pid, 1);
        child_pid = -1;
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
    but_extract->callback(abort_cb);
    Fl::add_timeout(img_duration, th);

    unlock();

    if ((fp = popen_mkvextract()) == NULL) {
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
        child_pid = -1;

        const char *l = (fclose(fp) == 0) ? "DONE" : "ERROR";

        lock();
        progress_box->label(l);
        unlock();
    }

    lock();

    dnd_area->activate();
    check_outdir->activate();
    but_outdir->activate();
    but_add->activate();
    but_extract->label("Extract");
    but_extract->callback(extract_cb);

    Fl::remove_timeout(th);
    rotate_img->image(NULL);

    unlock();

    if (chapters && extract_chapters) {
        if (same_as_source) {
            base = outdir_auto + fs::path(file).stem().string();
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
        base = outdir_auto + fs::path(file).stem().string();
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

static void clipboard_cb(Fl_Widget *)
{
    char *text = cmdBuff->text();
    Fl::copy(text, cmdBuff->length(), 1);
    free(text);
}

static void close_cmdWin_cb(Fl_Widget *)
{
    if (cmdWin && cmdWin->shown()) {
        cmdWin->hide();
    }
}

static void cmd_callback(Fl_Widget *)
{
    std::string command = create_extraction_command(false);
    cmdBuff->text(command.c_str());
    cmdWin->show();
}

static void extract_cb(Fl_Widget *)
{
    close_cmdWin_cb(NULL);

    if (pthread_create(&tcom, NULL, &run_extraction_command, NULL) == 0) {
        tcom_init = true;
    }
}

static void stop_threads()
{
    if (tcom_init) {
        pthread_cancel(tcom);
        tcom_init = false;
    }

    if (tinfo_init) {
        pthread_cancel(tinfo);
        tinfo_init = false;
    }

    if (child_pid > getpid()) {
        kill(child_pid, 1);
        child_pid = -1;
    }
}

static void abort_cb(Fl_Widget *)
{
    stop_threads();

    progress_box->label("STOPPED");
    check_outdir->activate();
    but_outdir->activate();
    but_add->activate();
    but_extract->label("Extract");
    but_extract->callback(extract_cb);

    Fl::remove_timeout(th);
    rotate_img->image(NULL);
}


static void check_outdir_cb(Fl_Widget *)
{
    if (same_as_source) {
        outdir_field->copy_label(outdir_manual.c_str());
        outdir_field->activate();
        same_as_source = false;
    } else {
        outdir_field->copy_label(outdir_auto.c_str());
        outdir_field->deactivate();
        same_as_source = true;
    }
}


static void browser_cb(Fl_Widget *)
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

static void close_cb(Fl_Widget *, void *)
{
    stop_threads();
    close_cmdWin_cb(NULL);
    win->hide();
}

static void mkvextract_init()
{
    /* timeout handler */
    th = rotate_th_callback;

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
    char buf[sizeof(svg_rotation_template) + 16];

    std::vector<const char *> v = { /* color values */
        "555", "999", "ddd", "000", "000", "000", "000", "000"
    };

    for (size_t i = 0; i < v.size(); i++) {
        if (i > 0) {
            /* rotate color entries */
            v.insert(v.begin(), v.back());
            v.pop_back();
        }

        snprintf(buf, sizeof(buf), svg_rotation_template,
            v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);

        img_rotate.push_back(new Fl_SVG_Image(NULL, buf));
        img_rotate.back()->resize(but_h, but_h);
    }
}

static void mkvextract_cleanup()
{
    stop_threads();

    if (win) delete win;
    if (fc) delete fc;
    if (cmdWin) delete cmdWin;

    while (!img_rotate.empty()) {
        delete img_rotate.back();
        img_rotate.pop_back();
    }
}


static void select_all_cb(Fl_Widget *, void *)
{
    browser->check_all();
    browser_cb(NULL);
}

static void select_none_cb(Fl_Widget *, void *)
{
    browser->check_none();
    browser_cb(NULL);
}

static void dismiss_cb(Fl_Widget *, void *)
{
}


int mkvextract(const char *in)
{
    const int w = 800, h = 480;
    const int but_w = 110;

    static Fl_Menu_Item context_menu[] = {
        { " Select all",      0, select_all_cb                         },
        { " Select none",     0, select_none_cb, NULL, FL_MENU_DIVIDER },
        { " Open file",       0, add_callback                          },
        { " Close program  ", 0, close_cb,       NULL, FL_MENU_DIVIDER },
        { " Dismiss",         0, dismiss_cb                            },
        { 0 }
    };

    /* init data */
    mkvextract_init();

    /* main window */
    Fl_Box *dummy;
    Fl_Group *g, *g_top, *g_inside1, *g_inside2;

    win = new Fl_Double_Window(w, h, "simple mkvextract GUI");
    win->callback(close_cb);
    {
        g = new Fl_Group(0, h - but_h*2 - 25, w, but_h*2 + 25);
        {
            but_extract = new Fl_Button(w - 10 - but_w, h - 10 - but_h, but_w, but_h, "Extract");
            but_extract->callback(extract_cb);
            but_extract->deactivate();

            but_cmd = new Fl_Button(but_extract->x() - 10 - but_w, but_extract->y(), but_w, but_h, "Command");
            but_cmd->callback(cmd_callback);
            but_cmd->deactivate();

            g_inside1 = new Fl_Group(0, but_extract->y(), w - 30 - 2*but_w, but_h);
            {
                progress_box = new Fl_Box(10, but_extract->y(), but_w, but_h);
                progress_box->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
                progress_box->box(FL_THIN_DOWN_BOX);

                rotate_img = new Fl_Box(but_w + 15, progress_box->y(), but_h, but_h);

                dummy = new Fl_Box(but_cmd->x() - 1, progress_box->y(), 1, 1);
                dummy->box(FL_NO_BOX);
            }
            g_inside1->resizable(dummy);
            g_inside1->end();

            g_inside2 = new Fl_Group(0, g->y(), g_inside1->w(), but_h);
            {
                outdir_field = new Fl_Box(10, but_extract->y() - but_h - 5, g_inside2->w() - 10, but_h, outdir_manual.c_str());
                outdir_field->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
                outdir_field->box(FL_THIN_DOWN_BOX);
            }
            g_inside2->resizable(outdir_field);
            g_inside2->end();

            check_outdir = new Fl_Check_Button(but_extract->x(), but_extract->y() - but_h - 5, but_w, but_h, " Use Source");
            check_outdir->deactivate();
            check_outdir->callback(check_outdir_cb);
            check_outdir->clear_visible_focus();

            but_outdir = new Fl_Button(but_cmd->x(), check_outdir->y(), but_w, but_h, "Destination");
            but_outdir->callback(browse_outdir_cb);
        }
        g->resizable(g_inside2);
        g->end();

        g_top = new Fl_Group(0, 0, w, but_h + 5);
        {
            but_add = new Fl_Button(w - 10 - but_w, 5, but_w, but_h, "Open file");
            but_add->callback(add_callback);

            infile_label = new Fl_Box(11, 5, but_add->x() - 20, but_h, "(drag and drop a Matroska file)");
            infile_label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
            infile_label->box(FL_THIN_DOWN_BOX);
            infile_label->labelsize(12);
        }
        g_top->resizable(infile_label);
        g_top->end();

        browser = new check_browser(10, but_add->y() + but_add->h() + 5, w - 20, h - but_h*3 - 35);
        browser->menu(context_menu);
        browser->callback(browser_cb);
        browser->clear_visible_focus();

        dnd_area = new dnd_box(browser->x(), browser->y(), browser->w(), browser->h());
        dnd_area->callback(dnd_callback);
    }
    win->end();
    win->resizable(browser);
    win->position((Fl::w() - win->w()) / 2, (Fl::h() - win->h()) / 2); /* center */
    win->size_range(512, 384, Fl::w(), Fl::h());

    /* cmd window */
    Fl_Text_Display *disp;
    Fl_Button *but_copy, *but_close;
    Fl_Box *dummy2;
    Fl_Group *g_cmd;

    cmdWin = new Fl_Double_Window(640, 320, "Command line");
    {
        cmdBuff = new Fl_Text_Buffer();
        disp = new Fl_Text_Display(15, 15, cmdWin->w() - 30, cmdWin->h() - 30 - but_h);
        disp->buffer(cmdBuff);
        disp->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 2);

        g_cmd = new Fl_Group(0, disp->h() + disp->y(), cmdWin->w(), cmdWin->h() - disp->h() - disp->y());
        {
            but_close = new Fl_Button(cmdWin->w() - 110 - 15, disp->h() + disp->y() + 6, 110, but_h, "Close");
            but_close->callback(close_cmdWin_cb);
            but_copy = new Fl_Button(but_close->x() - 150 - 5, but_close->y(), 150, but_h, "Copy to clipboard");
            but_copy->callback(clipboard_cb);
            dummy2 = new Fl_Box(but_copy->x() - 1, but_copy->y(), 1, 1);
        }
        g_cmd->end();
        g_cmd->resizable(dummy2);
    }
    cmdWin->position((Fl::w() - cmdWin->w()) / 2, (Fl::h() - cmdWin->h()) / 2); /* center */
    cmdWin->resizable(disp);

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

    mkv_file_info_create_thread();

    /* uncomment to test rotating animation */
    //Fl::add_timeout(img_duration, th);

    /* run */
    int rv = Fl::run();

    mkvextract_cleanup();

    return rv;
}

