/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018, djcj <djcj@gmx.de>
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
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Double_Window.H>
#include <FL/filename.H>

#include <cstdlib>
#include <iostream>
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
#include "misc.hpp"
#include "parsemkv.hpp"
#include "rotate.h"
#include "mkvtoolnix/icon.h"

#define LOCK    Fl::lock();
#define UNLOCK  Fl::unlock(); Fl::awake();

Fl_RGB_Image *img_arr[] =
{
  new Fl_PNG_Image(NULL, __0_png, __0_png_len),
  new Fl_PNG_Image(NULL, __1_png, __1_png_len),
  new Fl_PNG_Image(NULL, __2_png, __2_png_len),
  new Fl_PNG_Image(NULL, __3_png, __3_png_len),
  new Fl_PNG_Image(NULL, __4_png, __4_png_len),
  new Fl_PNG_Image(NULL, __5_png, __5_png_len),
  new Fl_PNG_Image(NULL, __6_png, __6_png_len),
  new Fl_PNG_Image(NULL, __7_png, __7_png_len)
};

const float img_duration = 0.1;  /* seconds */
const int last_frame = 7;
int current_frame = 0;

Fl_Double_Window *win;
Fl_Check_Browser *browser;
dnd_box *dnd_area;
Fl_Button *but_outdir, *but_add, *but_extract, *but_abort, *but_cmd;
Fl_Box *progress_box, *outdir_field, *infile_label, *rotate_img;
Fl_Check_Button *check_outdir;

pthread_t pt;
pid_t pid = -1;
bool chapters = false, same_as_source = false;
size_t count = 0, attach_count = 0;

std::string file, outdir_auto, outdir_manual, command;
std::vector<int> timestampIDs;
std::vector<std::string> outnames, args;


void error_exit(const char *title, const char *message)
{
  win->hide();
  fl_close = "Close";
  fl_message_title(title);
  fl_alert("%s", message);
  std::exit(1);
}

void get_mkv_file_info(void)
{
  std::vector<std::string> tracks, attachments, names1, names2;

CHECK_AGAIN:

  if (!file_is_matroska(file.c_str())) {
    fl_message_title("Warning");
    int rv = fl_choice("The selected file is not of type Matroska or WebM.\n"
                       "Do you want to continue anyway?",
                       "   Stop   ", "Continue", "Try again");
    if (rv == 0) {
      return;
    } else if (rv == 2) {
      goto CHECK_AGAIN;
    }
  }

  /* parse mkvinfo output */
  try {
    std::string error;
    if (!parsemkv(file, tracks, names1, attachments, names2, timestampIDs, chapters, error)) {
      fl_close = "   OK ";
      fl_message_title("Warning");
      fl_message("%s", error.c_str());
      return;
    }
  }
  catch (const std::overflow_error &e) {
    error_exit("parsemkv(): overflow error detected", e.what());
  }
  catch (const std::runtime_error &e) {
    error_exit("parsemkv(): runtime error detected", e.what());
  }
  catch (const std::exception &e) {
    error_exit("parsemkv(): exception thrown", e.what());
  }
  catch (...) {
    error_exit("parsemkv(): C++ runtime error", "an unknown C++ runtime error has occured");
  }

  /* get dirname */
  outdir_auto = file;
  std::size_t pos = outdir_auto.find_last_of('/');
  if (pos == std::string::npos) {
    outdir_auto = "";
  } else {
    outdir_auto.erase(pos + 1);
  }

  outnames.clear();
  browser->clear();
  count = tracks.size();
  attach_count = attachments.size();

  for (size_t i = 0; i < count; i++) {
    browser->add(tracks.at(i).c_str());
    outnames.push_back(names1.at(i));
  }

  for (size_t i = 0; i < attach_count; i++) {
    browser->add(attachments.at(i).c_str());
    outnames.push_back(names2.at(i));
  }

  if (chapters) {
    browser->add("Chapters");
  }
  browser->add("Video timestamps");
  browser->add("Tags");

  tracks.clear();
  names1.clear();
  attachments.clear();
  names2.clear();

  infile_label->copy_label(file.c_str());
  check_outdir->activate();
  win->redraw();
}

FILE *popen_mkvextract(void)
{
  enum { r = 0, w = 1 };
  int fd[2];

  if (pipe(fd) == -1) {
    return NULL;
  }

  if ((pid = fork()) != 0) {
    close(fd[w]);
    return fdopen(fd[r], "r");
  }

  size_t len = args.size();
  char **child_argv = new char *[len + 1];

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

  return NULL;
}

static void rotate_cb(Fl_Widget *)
{
  if (current_frame < last_frame) {
    current_frame++;
  } else {
    current_frame = 0;
  }
  rotate_img->image(img_arr[current_frame]);
  rotate_img->parent()->redraw();
  Fl::repeat_timeout(img_duration, (Fl_Timeout_Handler)rotate_cb);
}

static void dnd_callback(Fl_Widget *)
{
  std::string items(Fl::event_text());

  if (items.substr(0, 8) == "file:///") {
    std::size_t pos = items.find_first_of('\n');

    if (pos != std::string::npos) {
      items.erase(pos);
      decode_uri(items);
      file = items.erase(0, 7);
      dnd_area->deactivate();
      get_mkv_file_info();
      dnd_area->activate();
    }
  }
}

static void browse_outdir_cb(Fl_Widget *)
{
  const char *dir;

  if (getenv("KDE_FULL_SESSION")) {
    /* don't use GTK file chooser on KDE, there may be layout issues */
    dir = fl_dir_chooser("Select output directory", ".");
  } else {
    Fl_Native_File_Chooser *gtk = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    gtk->title("Select output directory");

    if (gtk->show() != 0) {
      return;
    }
    dir = gtk->filename();
  }

  if (!dir) {
    return;
  }

  outdir_manual = std::string(dir) + "/";

  if (!same_as_source) {
    outdir_field->copy_label(outdir_manual.c_str());
  }
}

static void add_cb(Fl_Widget *)
{
  const char *file_;

  /* open file */
  if (getenv("KDE_FULL_SESSION")) {
    /* don't use GTK file chooser on KDE, there may be layout issues */
    file_ = fl_file_chooser("Select a file", "*.mkv,*.mk3d,*.mka,*.mks,*.webm", NULL);
  } else {
    Fl_Native_File_Chooser *gtk = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
    gtk->title("Select a file");
    gtk->filter("*.mkv|*.mk3d|*.mka|*.mks|*.webm");

    if (gtk->show() != 0) {
      return;
    }
    file_ = gtk->filename();
  }

  if (!file_) {
    return;
  }

  file = std::string(file_);

  dnd_area->deactivate();
  get_mkv_file_info();
  dnd_area->activate();
}

extern "C" void *run_extraction_command(void *)
{
  FILE *fp;
  char *line = NULL;
  size_t n = 0;
  const char *l = "ERROR";

  const char *keyword = "#GUI#progress ";
  const size_t keyword_len = 14;

  LOCK

  dnd_area->deactivate();
  check_outdir->deactivate();
  but_outdir->deactivate();
  but_add->deactivate();
  but_abort->show();
  Fl::add_timeout(img_duration, (Fl_Timeout_Handler)rotate_cb);

  UNLOCK

  if ((fp = popen_mkvextract()) == NULL) {
    LOCK
    progress_box->label(l);
    UNLOCK
  }
  else
  {
    while (getline(&line, &n, fp) != -1) {
      if (line && strncmp(line, keyword, keyword_len) == 0) {
        LOCK
        /* trailing newline is ignored by label() */
        progress_box->copy_label(line + keyword_len);
        UNLOCK
      }
    }

    if (line) {
      free(line);
    }
    pid = -1;

    if (pclose(fp) == 0) {
      l = "DONE";
    }

    LOCK
    progress_box->label(l);
    UNLOCK
  }

  LOCK

  dnd_area->activate();
  check_outdir->activate();
  but_outdir->activate();
  but_add->activate();
  but_abort->hide();

  Fl::remove_timeout((Fl_Timeout_Handler)rotate_cb);
  rotate_img->image(NULL);

  UNLOCK

  return nullptr;
}

/* replace all instances of »'« with »'\''« and put
 * single quotes around the string */
static inline void quote_filename(std::string &s)
{
  size_t pos;
  const size_t newlen = 4;  /* strlen("'\\''") */

  for (pos = 0; (pos = s.find("'", pos)) != std::string::npos; pos += newlen) {
    s.replace(pos, 1, "'\\''");
  }
  s.insert(0, 1, '\'');
  s.push_back('\'');
}

bool create_extraction_command(bool extract)
{
  bool has_tracks = false, has_attach = false;
  std::string attach_dir, temp;
  size_t timestamps_entry, tags_entry, chapters_entry;

  if (file.empty() || file.length() == 0) {
    return false;
  }

  /* get basename without extension */
  std::string base = file;
  std::size_t pos = base.find_last_of('/');

  if (pos != std::string::npos) {
    base.erase(0, pos + 1);
  }

  if ((pos = base.find_last_of('.')) != std::string::npos) {
    std::string s = base.substr(pos);
    if (s == ".mkv" || s == ".mka" || s == ".mks" || s == ".mk3d" || s == ".webm") {
      base.erase(pos);
    }
  }

  if (extract) {
    CLEAR_VECTOR(args)
    args.push_back("mkvextract");
    args.push_back(file);
    args.push_back("--ui-language");
    args.push_back("en_US");
    args.push_back("--gui-mode");
  } else {
    temp = file;
    quote_filename(temp);
    command = "mkvextract " + temp;
  }

  if (same_as_source) {
    base = outdir_auto + base;
  } else {
    base = outdir_manual + base;
  }

  /* tracks */
  for (size_t i = 0; i < count; i++) {
    if (browser->checked(i+1)) {
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
        temp = ss.str();
        quote_filename(temp);
        command += " " + temp;
      }
    }
  }

  /* attachments */
  if (attach_count > 0) {
    attach_dir = base + " - Attachments/";

    for (size_t i = 0; i < attach_count; i++) {
      if (browser->checked(i+count+1)) {
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
          temp = ss.str();
          quote_filename(temp);
          command += " " + temp;
        }
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
      temp = base + " - chapters.xml";

      if (extract) {
        args.push_back("chapters");
        args.push_back(temp);
      } else {
        quote_filename(temp);
        command += " chapters " + temp;
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
        temp = ss.str();
        quote_filename(temp);
        command += " " + temp;
      }
    }
  }

  /* tags */
  if (browser->checked(tags_entry)) {
    temp = base + " - tags.xml";

    if (extract) {
      args.push_back("tags");
      args.push_back(temp);
    } else {
      quote_filename(temp);
      command += " tags " + temp;
    }
  }

  return true;
}

static void copy_command_cb(Fl_Widget *)
{
  if (create_extraction_command(false)) {
    Fl::copy(command.c_str(), command.size(), 1);
    fl_close = "   OK ";
    fl_message_title("Clipboard");
    fl_message("%s", "Extraction command successfully copied to clipboard.");
  }
}

static void extract_cb(Fl_Widget *) {
  if (create_extraction_command(true)) {
    pthread_create(&pt, NULL, &run_extraction_command, NULL);
  }
}

static void abort_cb(Fl_Widget *)
{
  if (pid > getpid()) {
    kill(pid, 1);
    pid = -1;
  }
  pthread_cancel(pt);

  progress_box->label("STOPPED");
  check_outdir->activate();
  but_outdir->activate();
  but_add->activate();
  but_abort->hide();

  Fl::remove_timeout((Fl_Timeout_Handler)rotate_cb);
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

static void close_cb(Fl_Widget *) {
  pthread_cancel(pt);
  win->hide();
}

int main(void)
{
  Fl_Box *dummy1, *dummy2;
  Fl_Group *g, *g_top, *g_inside1, *g_inside2;
  char *current_dir;

  const int w = 800,
    h = 600,
    but_w = 110,
    but_h = 28,
    label_fs = 12,
    min_w = 512,
    min_h = 384;

  int max_w = Fl::w();
  int max_h = Fl::h();

  /* set destination to current directory */
  if ((current_dir = get_current_dir_name()) != NULL) {
    outdir_manual = std::string(current_dir);
    if (outdir_manual[outdir_manual.size() - 1] != '/') {
      outdir_manual += "/";
    }
    free(current_dir);
  } else {
    outdir_manual = "/tmp/";
  }

  /* get and print FLTK version */
  int version = Fl::api_version();
  int major = version / 10000;
  int minor = (version % 10000) / 100;
  int patch = version % 100;

  std::cout << "using FLTK " << major << "." << minor << "." << patch << " - http://fltk.org/\n"
    << "window icon was taken from mkvtoolnix - https://mkvtoolnix.download/\n"
    << "source code available at https://github.com/darealshinji/mkvextract-gui" << std::endl;

  Fl::scheme("gtk+");
  Fl::visual(FL_DOUBLE|FL_INDEX);
  Fl_Window::default_icon(new Fl_PNG_Image(NULL, icon_png, icon_png_len));

  win = new Fl_Double_Window(w, h, "mkvextract GUI");
  win->callback(close_cb);
  {
    g = new Fl_Group(0, h - but_h*2 - 25, w, but_h*2 + 25);
    {
      but_extract = new Fl_Button(w - 10 - but_w, h - 10 - but_h, but_w, but_h, "Extract");
      but_extract->callback(extract_cb);
      but_extract->deactivate();

      but_abort = new Fl_Button(w - 10 - but_w, h - 10 - but_h, but_w, but_h, "Abort");
      but_abort->callback(abort_cb);
      but_abort->hide();

      but_cmd = new Fl_Button(but_extract->x() - 10 - but_w, h - 10 - but_h, but_w, but_h, "Command");
      but_cmd->callback(copy_command_cb);
      but_cmd->deactivate();

      check_outdir = new Fl_Check_Button(w - 10 - but_w, but_extract->y() - but_h - 5, but_w, but_h, " Use Source");
      check_outdir->deactivate();
      check_outdir->callback(check_outdir_cb);
      check_outdir->clear_visible_focus();

      but_outdir = new Fl_Button(check_outdir->x() - 10 - but_w, check_outdir->y(), but_w, but_h, "Destination");
      but_outdir->callback(browse_outdir_cb);

      g_inside1 = new Fl_Group(0, h - 10 - but_h, but_outdir->x() - 10, but_h);
      {
        progress_box = new Fl_Box(10, h - 10 - but_h, but_w, but_h);
        progress_box->box(FL_THIN_DOWN_BOX);

        rotate_img = new Fl_Box(but_w + 15, progress_box->y(), but_h, but_h);

        dummy1 = new Fl_Box(but_cmd->x() - 1, progress_box->y(), 1, 1);
        dummy1->box(FL_NO_BOX);
      }
      g_inside1->resizable(dummy1);
      g_inside1->end();

      g_inside2 = new Fl_Group(0, h - but_h*2 - 25, but_outdir->x() - 10, but_h);
      {
        outdir_field = new Fl_Box(10, but_outdir->y(), but_outdir->x() - 20, but_h, outdir_manual.c_str());
        outdir_field->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
        outdir_field->box(FL_THIN_DOWN_BOX);
      }
      g_inside2->resizable(outdir_field);
      g_inside2->end();
    }
    g->resizable(g_inside2);
    g->end();

    g_top = new Fl_Group(0, 0, w, but_h + 5);
    {
      but_add = new Fl_Button(w - 10 - but_w, 5, but_w, but_h, "Open file");
      but_add->callback(add_cb);

      dummy2 = new Fl_Box(but_add->x() - 1, 5, 1, 1);
      dummy2->box(FL_NO_BOX);

      infile_label = new Fl_Box(11, 5, but_add->x() - 20, but_h, "(drag and drop a Matroska file)");
      infile_label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
      infile_label->labelsize(label_fs);
    }
    g_top->resizable(dummy2);
    g_top->end();

    browser = new Fl_Check_Browser(10, but_add->y() + but_add->h() + 5, w - 20, h - but_h*3 - 35);
    browser->box(FL_THIN_DOWN_BOX);
    browser->color(fl_lighter(fl_lighter(FL_BACKGROUND_COLOR)));
    browser->clear_visible_focus();
    browser->callback(browser_cb);
    browser->when(FL_WHEN_CHANGED);

    dnd_area = new dnd_box(browser->x(), browser->y(), browser->w(), browser->h());
    dnd_area->callback(dnd_callback);
  }
  win->end();
  win->resizable(browser);
  win->position((max_w - win->w()) / 2, (max_h - win->h()) / 2); /* center */
  win->size_range(min_w, min_h, max_w, max_h);
  win->show();

  LOCK

  /* uncomment to test rotating animation */
  //Fl::add_timeout(img_duration, (Fl_Timeout_Handler)rotate_cb);

  return Fl::run();
}

