/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2020, djcj <djcj@gmx.de>
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
#include <FL/Fl_Text_Display.H>
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
#include "mkvtoolnix/icon.h"
#include "MyCheckBrowser.hpp"
#include "parsemkv.hpp"
#include "rotate.h"
#include "xml2ogm.hpp"

static Fl_RGB_Image *img_arr[] = {
  new Fl_PNG_Image(NULL, __0_png, __0_png_len),
  new Fl_PNG_Image(NULL, __1_png, __1_png_len),
  new Fl_PNG_Image(NULL, __2_png, __2_png_len),
  new Fl_PNG_Image(NULL, __3_png, __3_png_len),
  new Fl_PNG_Image(NULL, __4_png, __4_png_len),
  new Fl_PNG_Image(NULL, __5_png, __5_png_len),
  new Fl_PNG_Image(NULL, __6_png, __6_png_len),
  new Fl_PNG_Image(NULL, __7_png, __7_png_len)
};

/* needed function declarations */
static std::string create_extraction_command(bool extract);
static void abort_cb(Fl_Widget *);
static void extract_cb(Fl_Widget *);
static void rotate_cb(Fl_Widget *);

static const float img_duration = 0.1;  /* seconds */
static const int last_frame = 7;
static int current_frame = 0;

static Fl_Double_Window *win = NULL;
static int max_w, max_h;
static const int but_h = 28;

static Fl_Double_Window *cmdWin = NULL;
static Fl_Text_Buffer *cmdBuff;

static MyCheckBrowser *browser;
static dnd_box *dnd_area;
static Fl_Button *but_outdir, *but_add, *but_extract, *but_cmd;
static Fl_Box *progress_box, *outdir_field, *infile_label, *rotate_img;
static Fl_Check_Button *check_outdir;

static Fl_Timeout_Handler th = reinterpret_cast<Fl_Timeout_Handler>(rotate_cb);

static pthread_t tcom, tinfo;
static pid_t child_pid = -1;
static bool chapters = false, same_as_source = false, extract_chapters = false;
static size_t count = 0, attach_count = 0;

static std::string file, outdir_auto, outdir_manual;
static std::vector<int> timestampIDs;
static std::vector<std::string> outnames;
static std::vector<std::string> args;


static FILE *popen_mkvextract(void)
{
  enum { r = 0, w = 1 };
  int fd[2];
  size_t len = args.size();
  //char **child_argv;
  char *child_argv[len + 1];

  if (pipe(fd) == -1) {
    return NULL;
  }

  if ((child_pid = fork()) != 0) {
    close(fd[w]);
    return fdopen(fd[r], "r");
  }

  //len = args.size();
  //child_argv = new char *[len + 1];

  for (size_t i = 0; i < len; i++) {
    child_argv[i] = const_cast<char *>(args.at(i).c_str());
  }
  child_argv[len] = NULL;

  close(fd[r]);
  dup2(fd[w], 1);
  close(fd[w]);
  execvp("mkvextract", child_argv);

  //delete[] child_argv;
  _exit(127);

  return NULL;
}

/* replace all instances of »'« with »'\''« and put
 * single quotes around the string */
static std::string quote_filename(const std::string in)
{
  size_t pos;
  const size_t newlen = 4;  /* strlen("'\\''") == 4 */
  std::string str = in;

  for (pos = 0; (pos = str.find("'", pos)) != std::string::npos; pos += newlen) {
    str.replace(pos, 1, "'\\''");
  }
  str.insert(0, 1, '\'');
  str.push_back('\'');
  return str;
}

/* get basename without extension */
static std::string get_basename(void)
{
  std::string base = file;
  std::size_t pos;

  if ((pos = base.find_last_of('/')) != std::string::npos) {
    base.erase(0, pos + 1);
  }

  if ((pos = base.find_last_of('.')) != std::string::npos) {
    std::string s = base.substr(pos);
    if (s == ".mkv" || s == ".mka" || s == ".mks" || s == ".mk3d" || s == ".webm") {
      base.erase(pos);
    }
  }

  return base;
}

static bool file_is_matroska(void)
{
  FILE *fp;
  size_t len;
  unsigned char bytes[4] = {0};

  if (file.empty() || file == "" || !(fp = fopen(file.c_str(), "r"))) {
    return false;
  }

  len = fread(bytes, 1, sizeof(bytes), fp);

  if (ferror(fp) || len < sizeof(bytes)) {
    fclose(fp);
    return false;
  }
  fclose(fp);

  if (memcmp(bytes, "\x1A\x45\xDF\xA3", 4) == 0) {
    return true;
  }
  return false;
}

extern "C" void *get_mkv_file_info(void *)
{
  std::vector<std::string> tracks, attachments, names1, names2;
  std::string error;
  std::size_t pos;

  Fl::lock();

  dnd_area->deactivate();
  but_add->deactivate();

  Fl::unlock();
  Fl::awake();

CHECK_AGAIN:

  if (!file_is_matroska()) {
    Fl::lock();

    fl_message_title("Warning");
    int rv = fl_choice("The selected file is not of type Matroska or WebM.\n"
                       "Do you want to continue anyway?",
                       "   Stop   ", "Continue", "Try again");

    Fl::unlock();
    Fl::awake();

    if (rv == 0) {
      Fl::lock();

      dnd_area->activate();
      but_add->activate();

      Fl::unlock();
      Fl::awake();
      return nullptr;
    } else if (rv == 2) {
      goto CHECK_AGAIN;
    }
  }

  if (!parsemkv(quote_filename(file), tracks, names1, attachments, names2, timestampIDs, chapters, error)) {
    Fl::lock();

    fl_message_title("Error");
    fl_message("%s", error.c_str());
    dnd_area->activate();
    but_add->activate();

    Fl::unlock();
    Fl::awake();
    return nullptr;
  }

  /* get dirname */
  outdir_auto = file;

  if ((pos = outdir_auto.find_last_of('/')) == std::string::npos) {
    outdir_auto = "";
  } else {
    outdir_auto.erase(pos + 1);
  }

  outnames.clear();
  count = tracks.size();
  attach_count = attachments.size();

  Fl::lock();
  browser->clear();
  Fl::unlock();
  Fl::awake();

  for (size_t i = 0; i < count; i++) {
    Fl::lock();
    browser->add(tracks.at(i).c_str());
    Fl::unlock();
    Fl::awake();

    outnames.push_back(names1.at(i));
  }

  for (size_t i = 0; i < attach_count; i++) {
    Fl::lock();
    browser->add(attachments.at(i).c_str());
    Fl::unlock();
    Fl::awake();

    outnames.push_back(names2.at(i));
  }

  Fl::lock();

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

  Fl::unlock();
  Fl::awake();

  tracks.clear();
  names1.clear();
  attachments.clear();
  names2.clear();

  return nullptr;
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
  Fl::repeat_timeout(img_duration, th);
}

static void dnd_callback(Fl_Widget *)
{
  std::string items(Fl::event_text());
  std::size_t pos;

  if (items.substr(0, 8) == "file:///" &&
      (pos = items.find_first_of('\n')) != std::string::npos)
  {
    items.erase(pos);
    char *ch = strdup(items.c_str());
    fl_decode_uri(ch);
    file = ch + 7;
    free(ch);
    pthread_create(&tinfo, NULL, &get_mkv_file_info, NULL);
  }
}

static void browse_outdir_cb(Fl_Widget *)
{
  Fl_Native_File_Chooser *gtk;
  const char *dir;

  gtk = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
  gtk->title("Select output directory");

  if (gtk->show() != 0) {
    return;
  }
  dir = gtk->filename();

  if (!dir) {
    return;
  }

  outdir_manual = std::string(dir) + "/";

  if (!same_as_source) {
    outdir_field->copy_label(outdir_manual.c_str());
  }
}

static void add_cb(Fl_Widget *, void *)
{
  Fl_Native_File_Chooser *gtk = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
  gtk->title("Select a file");
  gtk->filter("*.mkv|*.mk3d|*.mka|*.mks|*.webm");

  if (gtk->show() == 0 && gtk->filename() != NULL) {
    file = std::string(gtk->filename());
    pthread_create(&tinfo, NULL, &get_mkv_file_info, NULL);
  }
}

extern "C" void *run_extraction_command(void *)
{
  FILE *fp;
  std::string base, xml, ogm;
  char *line = NULL;
  size_t n = 0;

  const char *keyword = "#GUI#progress ";
  const size_t keyword_len = 14;

  if (child_pid > getpid()) {
    kill(child_pid, 1);
    child_pid = -1;
  }

  if (system("mkvextract --version 2>/dev/null >/dev/null") != 0) {
    Fl::lock();

    fl_message_title("Error");
    fl_message("%s", "mkvextract doesn't seem to be in PATH!");

    Fl::unlock();
    Fl::awake();
    return nullptr;
  }

  create_extraction_command(true);

  Fl::lock();

  dnd_area->deactivate();
  check_outdir->deactivate();
  but_outdir->deactivate();
  but_add->deactivate();
  but_extract->label("Abort");
  but_extract->callback(abort_cb);
  Fl::add_timeout(img_duration, th);

  Fl::unlock();
  Fl::awake();

  if ((fp = popen_mkvextract()) == NULL) {
    Fl::lock();
    progress_box->label("ERROR");
    extract_chapters = false;
    Fl::unlock();
    Fl::awake();
  } else {
    while (getline(&line, &n, fp) != -1) {
      if (line && strncmp(line, keyword, keyword_len) == 0) {
        Fl::lock();
        /* trailing newline is ignored by label() */
        progress_box->copy_label(line + keyword_len);
        Fl::unlock();
        Fl::awake();
      }
    }

    if (line) {
      free(line);
    }
    child_pid = -1;

    const char *l = (fclose(fp) == 0) ? "DONE" : "ERROR";

    Fl::lock();
    progress_box->label(l);
    Fl::unlock();
    Fl::awake();
  }

  Fl::lock();

  dnd_area->activate();
  check_outdir->activate();
  but_outdir->activate();
  but_add->activate();
  but_extract->label("Extract");
  but_extract->callback(extract_cb);

  Fl::remove_timeout(th);
  rotate_img->image(NULL);

  Fl::unlock();
  Fl::awake();

  if (chapters && extract_chapters) {
    base = get_basename();

    if (same_as_source) {
      base = outdir_auto + base;
    } else {
      base = outdir_manual + base;
    }

    xml = base + " - chapters.xml";
    ogm = base + " - chapters.txt";

    if (!xml2ogm(xml.c_str(), ogm.c_str())) {
      Fl::lock();
      fl_message_title("Error");
      fl_message("%s", "Could not create OGM format chapters from XML!");
      Fl::unlock();
      Fl::awake();
    }
  }

  return nullptr;
}

static std::string create_extraction_command(bool extract)
{
  bool has_tracks = false, has_attach = false;
  std::string command, base, attach_dir;
  size_t timestamps_entry, tags_entry, chapters_entry;

  extract_chapters = false;
  args.clear();

  base = get_basename();

  if (extract) {
    args.push_back("mkvextract");
    args.push_back(file);
    args.push_back("--ui-language");
    args.push_back("en_US");
    args.push_back("--gui-mode");
  } else {
    //std::string s = file;
    //quote_filename(s);
    command = "mkvextract " + quote_filename(file);
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
        //std::string s = ss.str();
        //quote_filename(s);
        command += " " + quote_filename(ss.str());
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
          //std::string s = ss.str();
          //quote_filename(s);
          command += " " + quote_filename(ss.str());
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
      std::string s = base + " - chapters.xml";

      if (extract) {
        args.push_back("chapters");
        args.push_back(s);
        extract_chapters = true;
      } else {
        //quote_filename(s);
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
        //std::string s = ss.str();
        //quote_filename(s);
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
      //quote_filename(s);
      command += " tags " + quote_filename(s);
    }
  }

  return command;
}

static void clipboard_cb(Fl_Widget *) {
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

static void cmd_cb(Fl_Widget *)
{
  Fl_Text_Display *disp;
  Fl_Button *but_copy, *but_close;
  Fl_Box *dummy;
  Fl_Group *g;
  std::string command;

  if (!cmdWin) {
    cmdWin = new Fl_Double_Window(640, 320, "Command line");
    {
      cmdBuff = new Fl_Text_Buffer();
      disp = new Fl_Text_Display(15, 15, cmdWin->w() - 30, cmdWin->h() - 30 - but_h);
      disp->buffer(cmdBuff);
      disp->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 2);

      g = new Fl_Group(0, disp->h() + disp->y(), cmdWin->w(), cmdWin->h() - disp->h() - disp->y());
      {
        but_close = new Fl_Button(cmdWin->w() - 110 - 15, disp->h() + disp->y() + 6, 110, but_h, "Close");
        but_close->callback(close_cmdWin_cb);
        but_copy = new Fl_Button(but_close->x() - 150 - 5, but_close->y(), 150, but_h, "Copy to clipboard");
        but_copy->callback(clipboard_cb);
        dummy = new Fl_Box(but_copy->x() - 1, but_copy->y(), 1, 1);
      }
      g->end();
      g->resizable(dummy);
    }
    cmdWin->position((max_w - cmdWin->w()) / 2, (max_h - cmdWin->h()) / 2); /* center */
    cmdWin->resizable(disp);
  }

  command = create_extraction_command(false);
  cmdBuff->text(command.c_str());
  cmdWin->show();
}

static void extract_cb(Fl_Widget *) {
  close_cmdWin_cb(NULL);
  pthread_create(&tcom, NULL, &run_extraction_command, NULL);
}

static void abort_cb(Fl_Widget *)
{
  if (child_pid > getpid()) {
    kill(child_pid, 1);
    child_pid = -1;
  }
  pthread_cancel(tcom);

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
  pthread_cancel(tcom);
  pthread_cancel(tinfo);

  if (child_pid > getpid()) {
    kill(child_pid, 1);
    child_pid = -1;
  }

  close_cmdWin_cb(NULL);
  win->hide();
}

static void select_all_cb(Fl_Widget *, void *) {
  browser->check_all();
  browser_cb(NULL);
}

static void select_none_cb(Fl_Widget *, void *) {
  browser->check_none();
  browser_cb(NULL);
}

static void dismiss_cb(Fl_Widget *, void *) {
  /* do nothing; menu window will just close */
}

int main(int argc, char *argv[])
{
  Fl_Box *dummy;
  Fl_Group *g, *g_top, *g_inside1, *g_inside2;
  char *p;

  const int w = 800, h = 480, but_w = 110;

  Fl_Menu_Item context_menu[] = {
    { " Select all",      0, select_all_cb                         },
    { " Select none",     0, select_none_cb, NULL, FL_MENU_DIVIDER },
    { " Open file",       0, add_cb                                },
    { " Close program  ", 0, close_cb,       NULL, FL_MENU_DIVIDER },
    { " Dismiss",         0, dismiss_cb                            },
    { 0 }
  };

  if (argc > 1 && strcmp(argv[1], "--help") == 0) {
    std::cout << "usage: " << argv[0] << " [FILE]" << std::endl;
    return 0;
  }

  max_w = Fl::w();
  max_h = Fl::h();

  /* set destination to current directory */
  if ((p = get_current_dir_name()) != NULL) {
    outdir_manual = std::string(p);
    if (outdir_manual[outdir_manual.size() - 1] != '/') {
      outdir_manual.push_back('/');
    }
    free(p);
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

      but_cmd = new Fl_Button(but_extract->x() - 10 - but_w, but_extract->y(), but_w, but_h, "Command");
      but_cmd->callback(cmd_cb);
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
      but_add->callback(add_cb);

      infile_label = new Fl_Box(11, 5, but_add->x() - 20, but_h, "(drag and drop a Matroska file)");
      infile_label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
      infile_label->box(FL_THIN_DOWN_BOX);
      infile_label->labelsize(12);
    }
    g_top->resizable(infile_label);
    g_top->end();

    browser = new MyCheckBrowser(10, but_add->y() + but_add->h() + 5, w - 20, h - but_h*3 - 35);
    browser->menu(context_menu);
    browser->callback(browser_cb);
    browser->clear_visible_focus();

    dnd_area = new dnd_box(browser->x(), browser->y(), browser->w(), browser->h());
    dnd_area->callback(dnd_callback);
  }
  win->end();
  win->resizable(browser);
  win->position((max_w - win->w()) / 2, (max_h - win->h()) / 2); /* center */
  win->size_range(512, 384, max_w, max_h);

  if (argc > 1) {
    if (fl_filename_isdir(argv[1])) {
      fl_message_title("Error");
      fl_message("`%s' is a directory!", argv[1]);
    } else if (access(argv[1], R_OK) == 0) {
      if (argv[1][0] != '/' && (p = realpath(argv[1], NULL)) != NULL) {
        file = std::string(p);
        free(p);
      } else {
        file = std::string(argv[1]);
      }
    } else {
      fl_message_title("Error");
      fl_message("cannot read file `%s'", argv[1]);
    }
  }

  win->show();

  Fl::lock();

  if (!file.empty()) {
    pthread_create(&tinfo, NULL, &get_mkv_file_info, NULL);
  }

  /* uncomment to test rotating animation */
  //Fl::add_timeout(img_duration, th);

  return Fl::run();
}

