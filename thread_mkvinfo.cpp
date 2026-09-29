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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/fl_ask.H>

#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "check_browser.hpp"
#include "dnd.hpp"
#include "pipe_command.hpp"
#include "mkvextract.hpp"
#include "codecs.h"


struct track_info {
    std::string codec;
    std::string fps;
    std::string name;
    std::string lang;
    std::string w;
    std::string h;
    std::string freq;
    std::string channels;
};

struct attachment_info {
    std::string name;
    std::string size;
};



/* display attachment file sizes in Bytes, KiB, MiB or GiB */
static std::string human_readable_size(std::string &line)
{
    const long kb = 1024;
    const long mb = kb * 1024;
    const long gb = mb * 1024;

    long size = atol(line.c_str());

    if (size >= kb) {
        const char *unit;
        char buf[64];
        double d = size;

        if (size < mb) {
            d /= kb;
            unit = " KiB";
        } else if (size < gb) {
            d /= mb;
            unit = " MiB";
        } else {
            d /= gb;
            unit = " GiB";
        }

        snprintf(buf, sizeof(buf), "%.1f", d);

        return std::string(buf) + unit;
    }

    return line + " B";
}


static inline bool find(const std::string &haystack, char needle, size_t &pos) {
    return ((pos = haystack.find(needle)) != std::string::npos);
}


static void get_fps_value(std::string &line)
{
    constexpr std::string_view str{" frames/fields per second for a video track)"};
    size_t pos;

    if (find(line, '(', pos) && line.ends_with(str)) {
        line.erase(line.size() - str.size());
        line.erase(0, pos + 1);
    } else {
        line = "??";
    }
}


/* check if line begins with str, removes str if true */
static inline bool match(std::string &line, const std::string_view &str)
{
    if (line.starts_with(str)) {
        line.erase(0, str.size());
        return true;
    }

    return false;
}


static const char *type_string(char type)
{
    switch (type)
    {
    case 'V':
        return "video";
    case 'A':
        return "audio";
    case 'S':
        return "subtitles";
    default:
        return "other";
    }
}


void MKVextract::run_mkvinfo()
{
    std::string error;

    Fl::lock();
    m_dnd_area->deactivate();
    m_but_add->deactivate();
    Fl::unlock();
    Fl::awake();

    if (!parse_mkvinfo(error)) {
        fold_text(error);

        Fl::lock();

        fl_message_title("Error");
        fl_message("%s", error.c_str());
        m_dnd_area->activate();
        m_but_add->activate();

        Fl::unlock();
        Fl::awake();

        return;
    }

    /* save input file's dirname */
    m_outdir_source = dir_name(m_file);

    Fl::lock();

    /* activate "Select ..." menu entries */
    auto menu = m_browser->menu();
    menu->activate();
    menu->next()->activate();

    /* update widgets */
    m_infile_label->copy_label(m_file.c_str());
    m_infile_label->activate();
    m_use_source_path->activate();
    m_dnd_area->activate();
    m_but_add->activate();
    m_but_extract->deactivate();
    m_but_cmd->deactivate();
    m_progress_box->label(NULL);
    do_check_outdir();

    Fl::unlock();
    Fl::awake();
}


/* read output of mkvinfo */
bool MKVextract::parse_mkvinfo(std::string &error)
{
    pipe_command cmd;
    std::ifstream ifs;
    std::vector<struct track_info> tracks;
    std::vector<struct attachment_info> attach;

    error.clear();

    if (!command_in_path("mkvinfo")) {
        error = "mkvinfo doesn't seem to be in PATH!";
        return false;
    }

    const char *args[] = {
        "mkvinfo",
        "--no-bom",
        "--ui-language", "en_US",
        //"--gui-mode",
        //"--abort-on-warnings",
        m_file.c_str(),
        NULL
    };

    /* run mkvinfo */
    FILE *fp = cmd.pipe_open(const_cast<char **>(args));

    if (!fp) {
        error = "cannot read output from mkvinfo";
        return false;
    }

    enum { tnone = 0, tvideo = 1, taudio = 2 };
    unsigned short track_entry = tnone;
    bool has_chapters = false;

    std::string line;
    ssize_t nread;
    size_t n = 0;
    char *buf = NULL;
    auto_free af(buf);

    /* find begin of tracks info */
    while ((nread = getline(&buf, &n, fp)) != -1) {
        if (strcmp(buf, "|+ Tracks\n") == 0) {
            break;
        } else if (buf[0] != '+' && buf[0] != '|') {
            error = buf;
            return false;
        }
    }

    if (nread == -1) {
        error = "no tracks found";
        return false;
    }

    /* parse tracks */
    while ((nread = getline(&buf, &n, fp)) != -1) {
        if (buf[0] != '+' && buf[0] != '|') {
            error = buf;
            return false;
        }

        line = buf;

        if (line.ends_with('\n')) {
            line.pop_back();
        }

        if (line == "| + Track") {
            struct track_info track_info = { .lang = "UND" };
            tracks.push_back(track_info);
            track_entry = tnone;
            continue;
        }
        else if (match(line, "|  + Codec ID: ")) {
            tracks.back().codec = line;
            continue;
        }
        else if (match(line, "|  + Default duration: ")) {
            get_fps_value(line);
            tracks.back().fps = line;
            continue;
        }
        else if (match(line, "|  + Name: ")) {
            tracks.back().name = line;
            continue;
        }
        else if (match(line, "|  + Language: ")) {
            for (auto &c : line) { c = toupper(c); }
            tracks.back().lang = line;
            continue;
        }
        else if (line.starts_with("|+")) {
            /* end of track entries */
            break;
        }

        if (track_entry == tnone) {
            if (match(line, "|  + Audio track")) {
                track_entry = taudio;
                continue;
            }
            else if (match(line, "|  + Video track")) {
                track_entry = tvideo;
                continue;
            }
        }
        else if (track_entry == tvideo) {
            if (match(line, "|   + Pixel width: ")) {
                tracks.back().w = line;
                continue;
            }
            else if (match(line, "|   + Pixel height: ")) {
                tracks.back().h = line;
                continue;
            }
        }
        else if (track_entry == taudio) {
            if (match(line, "|   + Channels: ")) {
                if (line == "2") {
                    line = "stereo";
                } else {
                    line += " channels";
                }
                tracks.back().channels = line;
                continue;
            }
            else if (match(line, "|   + Sampling frequency: ")) {
                tracks.back().freq = line;
                continue;
            } else {
                /* no entry means mono */
                tracks.back().channels = "mono";
                continue;
            }
        }
    }

    /* parse attachments and chapters */
    if (nread != -1) {
        while (getline(&buf, &n, fp) != -1) {
            if (buf[0] != '+' && buf[0] != '|') {
                error = buf;
                return false;
            }

            line = buf;

            if (line.ends_with('\n')) {
                line.pop_back();
            }

            if (line == "| + Attached") {
                struct attachment_info info;
                attach.push_back(info);
                continue;
            }
            else if (match(line, "|  + File name: ")) {
                attach.back().name = line;
                continue;
            }
            else if (match(line, "|  + File data: size ")) {
                attach.back().size = human_readable_size(line);
                continue;
            }
            else if (line == "|+ Chapters") {
                has_chapters = true;
                break;
            }
        }
    }

    if (ferror(fp) != 0) {
        error = buf;
        return false;
    }

    /* update browser */
    Fl::lock();
    m_browser->clear();
    Fl::unlock();

    m_outnames.clear();

    for (size_t i = 0; i < tracks.size(); i++) {
        std::stringstream e; /* browser entry */
        std::stringstream filename;
        char c = tracks.at(i).codec[0];

        /* generic track infos */
        e << "Track " << i+1 << ": " << type_string(c) << " [" << tracks.at(i).codec << "]";
        if (!tracks.at(i).name.empty()) {
            e << " [" << tracks.at(i).name << "]";
        }
        e << " [" << tracks.at(i).lang << "]";

        if (c == 'V') {
            /* video infos */
            e << " [" << tracks.at(i).w << "x" << tracks.at(i).h << ", " << tracks.at(i).fps << " fps]";
            m_timestampIDs.push_back(i); /* extract ony timestamps of video streams */
        } else if (c == 'A') {
            /* audio infos */
            e << " [" << tracks.at(i).channels << ", " << tracks.at(i).freq << "Hz]";
        }

        filename << "track " << i+1 << " " << type_string(c);

        /* append the correct file extension depending on the codec ID */
        for (const auto &codec : mkv_codec_list) {
            if (tracks.at(i).codec == codec.id) {
                filename << '.' << codec.ext;
                break;
            }
        }

        Fl::lock();
        m_browser->add(e.str().c_str());
        Fl::unlock();

        m_outnames.push_back(filename.str());
    }

    for (size_t i = 0; i < attach.size(); i++) {
        std::stringstream e; /* browser entry */
        e << "Attachment " << i+1 << ": " << attach.at(i).name << " [" << attach.at(i).size << "]";

        Fl::lock();
        m_browser->add(e.str().c_str());
        Fl::unlock();

        m_outnames.push_back(attach.at(i).name);
    }

    m_track_count = tracks.size();
    m_attach_count = attach.size();
    m_timestamps_entry = 0;
    m_chapters_entry = 0;
    bool has_timestamps = m_timestampIDs.size() > 0;

    /* video timestamps */
    if (has_timestamps) {
        Fl::lock();
        m_browser->add("Video timestamps");
        m_timestamps_entry = m_browser->nitems();
        Fl::unlock();
    }

    /* chapters */
    if (has_chapters) {
        Fl::lock();
        m_browser->add("Chapters (xml + ogm/txt)");
        m_chapters_entry = m_browser->nitems();
        Fl::unlock();
    }

    /* tags */
    Fl::lock();
    m_browser->add("Tags");
    m_tags_entry = m_browser->nitems();
    Fl::unlock();

    Fl::awake();

    return true;
}

