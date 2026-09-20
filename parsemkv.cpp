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


/* display attachment file sizes in KiB, MiB, etc. */
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


static inline bool check_line(std::string &line, const std::string str)
{
    if (line.starts_with(str)) {
        line.erase(0, str.size());
        return true;
    }

    return false;
}


bool parsemkv(std::string &mkv_file, struct mkv_file_info &info, std::string &error)
{
    std::ifstream ifs;
    std::vector<struct track_info> tracks;
    std::vector<struct attachment_info> attachments;
    std::string line;
    char *buf = NULL;
    size_t n = 0;

    /* run mkvinfo */

    if (system("mkvinfo --version 2>/dev/null >/dev/null") != 0) {
        error = "mkvinfo doesn't seem to be in PATH!";
        return false;
    }

    const char *args[] = {
        "mkvinfo", "--no-bom", "--ui-language", "en_US", mkv_file.c_str(), NULL
    };

    pipe_command cmd(const_cast<char **>(args));
    FILE *fp = cmd.pipe_open();

    if (!fp) {
        error = "cannot read output from mkvinfo";
        return false;
    }

    /* check format */
    if (getline(&buf, &n, fp) == -1 ||
        strcmp(buf, "+ EBML head\n") != 0)
    {
        error = "malformed mkvinfo output";
        cmd.pipe_close();
        return false;
    }

    error.clear();

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

    bool tracks_begin = false;
    info.has_chapters = false;

    /* parse tracks */
    while (getline(&buf, &n, fp) != -1) {
        line = buf;

        if (line.ends_with('\n')) {
            line.pop_back();
        }

        if (tracks_begin) {
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
                for (size_t i = 0; i < line.size(); i++) {
                    line[i] = toupper(line[i]);
                }
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
        } else if (line == "|+ Tracks") {
          tracks_begin = true;
          continue;
        }
    }

    /* parse attachments and chapters */
    while (getline(&buf, &n, fp) != -1) {
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
            info.has_chapters = true;
            break;
        }
    }

    for (size_t i = 0; i < tracks.size(); i++) {
        std::stringstream strm1, strm2;
        std::string type;

        switch (tracks.at(i).codecid[0])
        {
        case 'V': type = "video"; break;
        case 'A': type = "audio"; break;
        case 'S': type = "subtitles"; break;
        default:  type = "other"; break;
        }

        strm1 << "Track " << i+1 << ": " << type << " [" << tracks.at(i).codecid << "]";
        if (!tracks.at(i).name.empty()) {
            strm1 << " [" << tracks.at(i).name << "]";
        }
        strm1 << " [" << tracks.at(i).language << "]";

        if (type == "video") {
            strm1 << " [" << tracks.at(i).width << "x" << tracks.at(i).height;
            strm1 << ", " << tracks.at(i).duration << " fps]";

            /* for now only extract timestamps of video streams */
            info.timestampIDs.push_back(i);
        }
        else if (type == "audio") {
            strm1 << " [" << tracks.at(i).channels << ", " << tracks.at(i).freq << "Hz]";
        }

        strm2 << "track_" << i+1 << "_" << type;

        /* append the correct file extension depending on the codec ID */
        for (const auto &e : mkv_codec_list) {
            if (tracks.at(i).codecid == e.id) {
                strm2 << '.' << e.ext;
                break;
            }
        }

        struct infos nfo = { strm1.str(), strm2.str() };
        info.tracks.push_back(nfo);
    }

    for (size_t i = 0; i < attachments.size(); i++) {
        std::stringstream strm;

        strm << "Attachment " << i+1 << ": " << attachments.at(i).filename;
        strm << " [" << attachments.at(i).filesize << "]";

        struct infos nfo = { strm.str(), attachments.at(i).filename };
        info.attachments.push_back(nfo);
    }

    return true;
}

