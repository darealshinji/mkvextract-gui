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
#include <FL/Fl_Double_Window.H>
#include <FL/filename.H>

#include <cstdlib>
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

#define LOCK    Fl::lock();
#define UNLOCK  Fl::unlock(); Fl::awake();

Fl_Double_Window *win;
Fl_Check_Browser *browser;
dnd_box *dnd_area;
Fl_Button *but_outdir, *but_add, *but_extract, *but_abort;
Fl_Box *progress_box, *outdir_field, *infile_label;
Fl_Check_Button *check_outdir;

pthread_t pt;
pid_t pid = -1;
bool chapters = false, same_as_source = false;
size_t count = 0, attach_count = 0;

std::string file, outdir, outdir_auto, outdir_manual, command;
std::vector<int> timestampIDs;
std::vector<std::string> outnames;


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
      fl_close = "  OK ";
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

  browser->clear();
  outnames.clear();
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
  const char *keyword_pid = "#PID ";
  const size_t keyword_pid_len = 5;

  LOCK
  dnd_area->deactivate();
  check_outdir->deactivate();
  but_outdir->deactivate();
  but_add->deactivate();
  but_abort->show();
  UNLOCK

  if ((fp = popen(command.c_str(), "re")) == NULL) {
    LOCK
    progress_box->label(l);
    UNLOCK
  }
  else
  {
    while (getline(&line, &n, fp) != -1) {
      if (line) {
        if (strncmp(line, keyword, keyword_len) == 0) {
          LOCK
          /* trailing newline is ignored by label() */
          progress_box->copy_label(line + keyword_len);
          UNLOCK
        }
        else if (pid < 1 && strncmp(line, keyword_pid, keyword_pid_len) == 0) {
          pid = (pid_t) atol(line + keyword_pid_len);
        }
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
  UNLOCK

  return nullptr;
}

bool create_extraction_command(void)
{
  std::stringstream ss_tracks, ss_attach, ss_timestamps;
  std::string attach_dir;
  size_t timestamps_entry, tags_entry, chapters_entry;

  if (file.empty() || file.size() == 0) {
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

  command = "mkvextract \"" + file + "\" --ui-language en_US --gui-mode";
  outdir = same_as_source ? outdir_auto : outdir_manual;

  for (size_t i = 0; i < count; i++) {
    if (browser->checked(i+1)) {
      ss_tracks << " " << i << ":\"" << outdir << base << " - " << outnames.at(i) << "\"";
    }
  }

  if (attach_count > 0) {
    attach_dir = outdir + base + " - Attachments/";
    for (size_t i = 0; i < attach_count; i++) {
      if (browser->checked(i+count+1)) {
        ss_attach << " " << i+1 << ":\"" << attach_dir << outnames.at(i+count) << "\"";
      }
    }
  }

  if (ss_tracks.str().size() > 0) {
    command += " tracks" + ss_tracks.str();
  }

  if (ss_attach.str().size() > 0) {
    command += " attachments" + ss_attach.str();
    mkdir(attach_dir.c_str(), 0775);
  }

  timestamps_entry = count + attach_count + 1;
  tags_entry = count + attach_count + 2;

  if (chapters) {
    chapters_entry = count + attach_count + 1;
    timestamps_entry++;
    tags_entry++;
    if (browser->checked(chapters_entry)) {
      command += " chapters \"" + outdir + base + " - chapters.xml\"";
    }
  }

  if (browser->checked(timestamps_entry) && timestampIDs.size() > 0) {
    std::stringstream ss_tmp;
    for (size_t i = 0; i < timestampIDs.size(); i++) {
      int id = timestampIDs.at(i);
      ss_tmp << " " << id << ":\"" << outdir << base
        << " - track_" << id+1 << "_video_timestamps_v2.txt\"";
    }
    command += " timestamps_v2" + ss_tmp.str();
  }

  if (browser->checked(tags_entry)) {
    command += " tags \"" + outdir + base + " - tags.xml\"";
  }

  command += " & echo \"#PID $!\"";

  return true;
}

static void extract_cb(Fl_Widget *) {
  if (create_extraction_command()) {
    pthread_create(&pt, NULL, &run_extraction_command, NULL);
  }
}

static void abort_cb(Fl_Widget *)
{
  if (pid > 1) {
    kill(pid, 1);
    pid = -1;
  }
  pthread_cancel(pt);
  progress_box->label("STOPPED");
  check_outdir->activate();
  but_outdir->activate();
  but_add->activate();
  but_abort->hide();
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
    progress_box->label("READY");
  } else {
    but_extract->deactivate();
    progress_box->label("");
  }
}

static void close_cb(Fl_Widget *) {
  pthread_cancel(pt);
  win->hide();
}

int main(void)
{
  std::stringstream ss;
  std::string title;
  Fl_Box *dummy;
  Fl_Group *g, *g_inside1, *g_inside2;
  char *current_dir;

  const int w = 640
  ,         h = 480
  ,         but_w = 100
  ,         but_h = 26
  ,         label_fs = 12;

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

  /* get FLTK version for window title */
  int version = Fl::api_version();
  int major = version / 10000;
  int minor = (version % 10000) / 100;
  int patch = version % 100;
  ss << "mkvextract GUI (using FLTK " << major << "." << minor << "." << patch << ")";

  Fl::scheme("gtk+");

  win = new Fl_Double_Window(w, h);
  win->copy_label(ss.str().c_str());
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

      but_add = new Fl_Button(but_extract->x() - 10 - but_w, h - 10 - but_h, but_w, but_h, "Open file");
      but_add->callback(add_cb);

      check_outdir = new Fl_Check_Button(w - 10 - but_w, but_extract->y() - but_h - 5, but_w, but_h, "Use Source");
      check_outdir->deactivate(); /* activate when a file was added */
      check_outdir->callback(check_outdir_cb);
      check_outdir->clear_visible_focus();

      but_outdir = new Fl_Button(check_outdir->x() - 10 - but_w, check_outdir->y(), but_w, but_h, "Destination");
      but_outdir->callback(browse_outdir_cb);

      g_inside1 = new Fl_Group(0, h - 10 - but_h, but_outdir->x() - 10, but_h);
      {
        progress_box = new Fl_Box(10, h - 10 - but_h, but_w, but_h);
        progress_box->box(FL_THIN_DOWN_BOX);

        dummy = new Fl_Box(but_add->x() - 1, h - 20 - but_h, 1, 1);
        dummy->box(FL_NO_BOX);
      }
      g_inside1->resizable(dummy);
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

    infile_label = new Fl_Box(11, 5, w, 20, "drag and drop a Matroska file");
    infile_label->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
    infile_label->labelsize(label_fs);

    browser = new Fl_Check_Browser(10, infile_label->y() + infile_label->h(), w - 20, h - infile_label->h() - but_h*2 - 30);
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
  win->size_range(400, 300, max_w, max_h);
  win->show();

  LOCK

  return Fl::run();
}

