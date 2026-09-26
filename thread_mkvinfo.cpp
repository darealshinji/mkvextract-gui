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

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mkvextract.hpp"
#include "pipe_command.hpp"
#include "codecs.h"


struct track_info {
    std::string codecid;
    std::string duration;
    std::string name;
    std::string language;
    std::string width;
    std::string height;
    std::string freq;
    std::string channels;
};

struct attachment_info {
    std::string filename;
    std::string filesize;
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
    const char str[] = " frames/fields per second for a video track)";
    size_t pos;

    if (find(line, '(', pos) && line.ends_with(str)) {
        line.erase(line.size() - sizeof(str) - 1);
        line.erase(0, pos + 1);
    } else {
        line = "??";
    }
}


static inline bool check_line(std::string &line, const std::string &str)
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


/* this function is called by pthread_create() */
void *MKVextract::thread_run_mkvinfo(void *p)
{
    reinterpret_cast<MKVextract *>(p)->run_mkvinfo();
    return NULL;
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
    std::ifstream ifs;
    std::vector<struct track_info> tracks;
    std::vector<struct attachment_info> attachments;

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
    pipe_command cmd(const_cast<char **>(args));
    FILE *fp = cmd.pipe_open();

    if (!fp) {
        error = "cannot read output from mkvinfo";
        return false;
    }

    const std::string
        S_codecid =  "|  + Codec ID: ",
        S_duration = "|  + Default duration: ",
        S_name =     "|  + Name: ",
        S_language = "|  + Language: ",
        S_tvideo =   "|  + Video track",
        S_taudio =   "|  + Audio track",
        S_width =    "|   + Pixel width: ",
        S_height =   "|   + Pixel height: ",
        S_freq =     "|   + Sampling frequency: ",
        S_channels = "|   + Channels: ",
        S_filename = "|  + File name: ",
        S_filesize = "|  + File data: size ";

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
            struct track_info track_info = { .language = "UND" };
            tracks.push_back(track_info);
            track_entry = tnone;
            continue;
        }
        else if (check_line(line, S_codecid)) {
            tracks.back().codecid = line;
            continue;
        }
        else if (check_line(line, S_duration)) {
            get_fps_value(line);
            tracks.back().duration = line;
            continue;
        }
        else if (check_line(line, S_name)) {
            tracks.back().name = line;
            continue;
        }
        else if (check_line(line, S_language)) {
            for (auto &c : line) { c = toupper(c); }
            tracks.back().language = line;
            continue;
        }
        else if (line.starts_with("|+")) {
            /* end of track entries */
            break;
        }

        if (track_entry == tnone) {
            if (check_line(line, S_taudio)) {
                track_entry = taudio;
                continue;
            }
            else if (check_line(line, S_tvideo)) {
                track_entry = tvideo;
                continue;
            }
        }
        else if (track_entry == tvideo) {
            if (check_line(line, S_width)) {
                tracks.back().width = line;
                continue;
            }
            else if (check_line(line, S_height)) {
                tracks.back().height = line;
                continue;
            }
        }
        else if (track_entry == taudio) {
            if (check_line(line, S_channels)) {
                if (line == "2") {
                    line = "stereo";
                } else {
                    line += " channels";
                }
                tracks.back().channels = line;
                continue;
            }
            else if (check_line(line, S_freq)) {
                tracks.back().freq = line;
                continue;
            }

            /* no entry means mono */
            tracks.back().channels = "mono";
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
                attachments.push_back(info);
                continue;
            }
            else if (check_line(line, S_filename)) {
                attachments.back().filename = line;
                continue;
            }
            else if (check_line(line, S_filesize)) {
                attachments.back().filesize = human_readable_size(line);
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
        std::stringstream entry, filename;
        char ch = tracks.at(i).codecid[0];

        /* generic track infos */
        entry << "Track " << i+1 << ": " << type_string(ch);
        entry << " [" << tracks.at(i).codecid << "]";
        if (!tracks.at(i).name.empty()) {
            entry << " [" << tracks.at(i).name << "]";
        }
        entry << " [" << tracks.at(i).language << "]";

        if (ch == 'V') {
            /* video infos */
            entry << " [" << tracks.at(i).width << "x" << tracks.at(i).height;
            entry << ", " << tracks.at(i).duration << " fps]";
            m_timestampIDs.push_back(i); /* for now only extract timestamps of video streams */
        } else if (ch == 'A') {
            /* audio infos */
            entry << " [" << tracks.at(i).channels << ", " << tracks.at(i).freq << "Hz]";
        }

        filename << "track_" << i+1 << "_" << type_string(ch);

        /* append the correct file extension depending on the codec ID */
        for (const auto &e : mkv_codec_list) {
            if (tracks.at(i).codecid == e.id) {
                filename << '.' << e.ext;
                break;
            }
        }

        Fl::lock();
        m_browser->add(entry.str().c_str());
        Fl::unlock();

        m_outnames.push_back(filename.str());
    }

    for (size_t i = 0; i < attachments.size(); i++) {
        std::stringstream entry;
        entry << "Attachment " << i+1 << ": " << attachments.at(i).filename;
        entry << " [" << attachments.at(i).filesize << "]";

        Fl::lock();
        m_browser->add(entry.str().c_str());
        Fl::unlock();

        m_outnames.push_back(attachments.at(i).filename);
    }

    m_track_count = tracks.size();
    m_attach_count = attachments.size();
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

