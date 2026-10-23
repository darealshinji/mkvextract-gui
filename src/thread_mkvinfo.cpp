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

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stop_token>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "check_browser.hpp"
#include "dnd_window.hpp"
#include "pipe_command.hpp"
#include "mkvextract.hpp"


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

struct codec_specs {
    const char *id;
    const char *ext;
};


enum {
    INFO_OK,
    INFO_ERROR,
    INFO_STOP_REQUEST,

    TRACK_AUDIO,
    TRACK_VIDEO,
    TRACK_UNKNOWN
};



/* count objects in array */
template <typename T, size_t N>
size_t count_of(T(&)[N]) {
    return std::extent<T[N]>::value;
}


/* list of codec IDs and file extensions */
static struct codec_specs mkv_codec_list[] =
{
#include "codecs.i"
};


/* display attachment file sizes in human readable values */
static std::string human_readable_size(std::string &line)
{
    long size;

    try {
        size = std::stol(line);
    } catch(...) {
        return line + " B";
    }

    const long kb = 1024;
    const long mb = 1024 * kb;
    const long gb = 1024 * mb;

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


static bool find(const std::string &haystack, char needle, size_t &pos) {
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
static bool match(std::string &line, const std::string_view &str)
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


static bool parse_track_data(std::string &line,
                             std::vector<struct track_info> &tracks,
                             unsigned short &entry)
{
    if (line == "| + Track") {
        struct track_info track_info = { .lang = "UND" };
        tracks.push_back(track_info);
        entry = TRACK_UNKNOWN;
    }
    else if (match(line, "|  + Codec ID: ")) {
        tracks.back().codec = line;
    }
    else if (match(line, "|  + Default duration: ")) {
        get_fps_value(line);
        tracks.back().fps = line;
    }
    else if (match(line, "|  + Name: ")) {
        tracks.back().name = line;
    }
    else if (match(line, "|  + Language: ")) {
        for (auto &c : line) { c = toupper(c); }
        tracks.back().lang = line;
    }
    else if (line.starts_with("|+")) {
        /* end of track entries */
        return false;
    }
    else if (entry == TRACK_UNKNOWN) {
        if (match(line, "|  + Audio track")) {
            entry = TRACK_AUDIO;
        } else if (match(line, "|  + Video track")) {
            entry = TRACK_VIDEO;
        }
    }
    else if (entry == TRACK_VIDEO) {
        if (match(line, "|   + Pixel width: ")) {
            tracks.back().w = line;
        } else if (match(line, "|   + Pixel height: ")) {
            tracks.back().h = line;
        }
    }
    else if (entry == TRACK_AUDIO) {
        if (match(line, "|   + Channels: ")) {
            if (line == "2") {
                line = "stereo";
            } else {
                line += " channels";
            }
            tracks.back().channels = line;
        }
        else if (match(line, "|   + Sampling frequency: ")) {
            tracks.back().freq = line;
        } else {
            /* no entry means mono */
            tracks.back().channels = "mono";
        }
    }

    return true;
}


void MKVextract::run_mkvinfo(std::stop_token stoken)
{
    std::string error;

    Fl::lock();
    m_win->dnd_enabled(false);
    m_but_add->deactivate();
    Fl::unlock();
    Fl::awake();

    /* run mkvinfo and parse the output */
    int rv = parse_mkvinfo(stoken, error);

    if (rv == INFO_ERROR) {
        /* mkvinfo has returned with an error */
        utils::fold_text(error);

        Fl::lock();

        fl_message_title("Error");
        fl_message("%s", error.c_str());
        m_but_add->activate();
        m_win->dnd_enabled(true);

        Fl::unlock();
        Fl::awake();

        return;
    } else if (rv == INFO_STOP_REQUEST) {
        return; /* multithreading stop was requested */
    }

    /* save input file's dirname */
    m_outdir_source = std::filesystem::path(m_file).parent_path().string();

    if (!m_outdir_source.ends_with('/')) {
        m_outdir_source += '/';
    }

    Fl::lock();

    /* update widgets */
    auto sel_all = m_browser->menu();
    auto sel_none = sel_all->next();
    auto sel_attach = sel_none->next();
    sel_all->activate();
    sel_none->deactivate();

    if (m_attach_count == 0) {
        sel_attach->deactivate();
    } else {
        sel_attach->activate();
    }

    m_infile_label->copy_label(m_file.c_str());
    m_use_source_path->activate();
    m_but_add->activate();

    do_update_browser();
    do_check_outdir();

    m_win->dnd_enabled(true);

    Fl::unlock();
    Fl::awake();
}


/* read output of mkvinfo */
int MKVextract::parse_mkvinfo(std::stop_token &stoken, std::string &error)
{
    std::ifstream ifs;
    std::vector<struct track_info> tracks;
    std::vector<struct attachment_info> attach;

    unsigned short track_entry = TRACK_UNKNOWN;
    bool has_chapters = false;
    bool has_tracks = false;
    std::string line;
    int rv = INFO_OK;

    /* perform these checks at the top of each while loop */
    auto return_from_loop = [&] ()
    {
        /* multithreading stop was requested */
        if (stoken.stop_requested()) {
            rv = INFO_STOP_REQUEST;
            return true;
        }

        /* mkvinfo has returned an error message */
        if (!line.starts_with('+') && !line.starts_with('|')) {
            error = line;
            rv = INFO_ERROR;
            return true;
        }

        return false;
    };

    error.clear();

    if (!utils::command_in_path("mkvinfo")) {
        error = "mkvinfo doesn't seem to be in PATH!";
        return INFO_ERROR;
    }

    /* check before starting a pipe */
    if (stoken.stop_requested()) {
        return INFO_STOP_REQUEST;
    }

    /* run mkvinfo */
    const char *args[] = {
        "mkvinfo",
        "--no-bom",
        "--ui-language", "en_US",
        //"--gui-mode",
        //"--abort-on-warnings",
        m_file.c_str(),
        NULL
    };

    pipe_command cmd(args, count_of(args));

    if (!cmd.open_pipe()) {
        error = "cannot read output from mkvinfo";
        return INFO_ERROR;
    }

    /* find tracks info */
    while (cmd.getline(line)) {
        if (return_from_loop()) {
            return rv;
        } else if (line == "|+ Tracks") {
            /* begin of track entries */
            has_tracks = true;
            break;
        }
    }

    if (!has_tracks) {
        error = "no tracks found";
        return INFO_ERROR;
    }

    /* parse tracks */
    while (cmd.getline(line)) {
        if (return_from_loop()) {
            return rv;
        } else if (!parse_track_data(line, tracks, track_entry)) {
            /* end of track entries */
            break;
        }
    }

    /* parse attachments and chapters */
    while (cmd.getline(line)) {
        if (return_from_loop()) {
            return rv;
        }
        else if (line == "| + Attached") {
            struct attachment_info info;
            attach.push_back(info);
        }
        else if (match(line, "|  + File name: ")) {
            attach.back().name = line;
        }
        else if (match(line, "|  + File data: size ")) {
            attach.back().size = human_readable_size(line);
        }
        else if (line == "|+ Chapters") {
            has_chapters = true;
            break;
        }
    }

    if (cmd.error()) {
        error = line;
        return INFO_ERROR;
    }

    /* last check; from here on the browser and
     * member variables are updated */
    if (stoken.stop_requested()) {
        return INFO_STOP_REQUEST;
    }

    /* update browser */
    Fl::lock();
    m_browser->clear();
    Fl::unlock();

    m_outnames.clear();

    /* tracks */
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

    /* attachments */
    for (size_t i = 0; i < attach.size(); i++) {
        std::stringstream e; /* browser entry */
        e << "Attachment " << i+1 << ": " << attach.at(i).name;
        e << " [" << attach.at(i).size << "]";

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
    m_browser->redraw();
    Fl::unlock();

    Fl::awake();

    return INFO_OK;
}

