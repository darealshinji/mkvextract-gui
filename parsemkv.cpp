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

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mkvextract.hpp"
#include "codecs.h"


inline static bool checkLine(std::string &line, const std::string str)
{
    if (line.substr(0, str.size()) == str) {
        line.erase(0, str.size());
        return true;
    }

    return false;
}

template<class T, typename U>
void replace_last_entry(T &vec, U &str) {
    vec.pop_back();
    vec.push_back(str);
}


bool parsemkv(std::string &mkv_file
,             std::vector<std::string> &trackInfos
,             std::vector<std::string> &trackFilenames
,             std::vector<std::string> &attachmentInfos
,             std::vector<std::string> &attachmentFilenames
,             std::vector<int> &timestampIDs
,             bool &has_chapters
,             std::string &error)
{
    std::ifstream ifs;
    std::vector<std::string> codecid, duration, name, language, width, height,
        freq, channels, filename, mime, fdata;
    std::string line;
    char *buf = NULL;
    pid_t pid;
    FILE *fp;
    size_t n = 0;

    /* run mkvinfo */

    if (system("mkvinfo --version 2>/dev/null >/dev/null") != 0) {
        error = "mkvinfo doesn't seem to be in PATH!";
        return false;
    }

    char *args[] = {
        const_cast<char *>("mkvinfo"),
        const_cast<char *>("--no-bom"),
        const_cast<char *>("--ui-language"),
        const_cast<char *>("en_US"),
        const_cast<char *>(mkv_file.c_str()),
        NULL
    };

    if ((fp = popen_vp(args, pid)) == NULL) {
        error = "mkvinfo has returned an error";
        return false;
    }

    if (getline(&buf, &n, fp) == -1 || !buf || strcmp(buf, "+ EBML head\n") != 0) {
        error = "malformed mkvinfo output";
        fclose(fp);
        return false;
    }

    error = "";

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
        S_mime =     "|  + MIME type: ",
        S_fdata =    "|  + File data: size ";

    enum { tnone = 0, tvideo = 1, taudio = 2 };
    unsigned short track_entry = tnone;

    bool tracks_begin = false;
    has_chapters = false;

    trackInfos.clear();
    trackFilenames.clear();
    attachmentInfos.clear();
    attachmentFilenames.clear();
    timestampIDs.clear();

    /* parse tracks */
    while (getline(&buf, &n, fp) != -1) {
        line = buf;

        if (line.ends_with('\n')) {
            line.pop_back();
        }

        if (tracks_begin) {
            if (line == "| + Track") {
                codecid.push_back("");
                duration.push_back("");
                name.push_back("");
                language.push_back("und");
                width.push_back("");
                height.push_back("");
                freq.push_back("");
                channels.push_back("");
                track_entry = tnone;
                continue;
            }
            else if (checkLine(line, S_codecid)) {
                replace_last_entry(codecid, line);
                continue;
            }
            else if (checkLine(line, S_duration)) {
                replace_last_entry(duration, line);
                continue;
            }
            else if (checkLine(line, S_name)) {
                replace_last_entry(name, line);
                continue;
            }
            else if (checkLine(line, S_language)) {
                replace_last_entry(language, line);
                continue;
            }
            else if (line.starts_with("|+")) {
                /* end of track entries */
                break;
            }

            if (track_entry == tnone) {
                if (checkLine(line, S_taudio)) {
                    track_entry = taudio;
                    continue;
                }
                else if (checkLine(line, S_tvideo)) {
                    track_entry = tvideo;
                    continue;
                }
            }
            else if (track_entry == tvideo) {
                if (checkLine(line, S_width)) {
                    replace_last_entry(width, line);
                    continue;
                }
                else if (checkLine(line, S_height)) {
                    line = "x" + line;
                    replace_last_entry(height, line);
                    continue;
                }
            }
            else if (track_entry == taudio) {
                if (checkLine(line, S_channels)) {
                    line += " channels";
                    replace_last_entry(channels, line);
                    continue;
                }
                else if (checkLine(line, S_freq)) {
                    line = ", " + line + "Hz";
                    replace_last_entry(freq, line);
                    continue;
                }

                /* no entry means mono */
                replace_last_entry(channels, "1 channel");
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
            filename.push_back("");
            mime.push_back("");
            fdata.push_back("");
            continue;
        }
        else if (checkLine(line, S_filename)) {
            replace_last_entry(filename, line);
            continue;
        }
        else if (checkLine(line, S_mime)) {
            replace_last_entry(mime, line);
            continue;
        }
        else if (checkLine(line, S_fdata)) {
            line += " bytes";
            replace_last_entry(fdata, line);
            continue;
        }
        else if (line == "|+ Chapters") {
            has_chapters = true;
            break;
        }
    }

    fclose(fp);

    for (size_t i = 0; i < codecid.size(); i++) {
        std::stringstream ss1, ss2;
        std::string type;

        switch (codecid.at(i)[0]) {
        case 'V': type = "video"; break;
        case 'A': type = "audio"; break;
        case 'S': type = "subtitles"; break;
        default:  type = "other"; break;
        }

        ss1 << "Track " << i+1 << " "
            "[TID " << i << "] "
            "[" << type << "] "
            "[" << codecid.at(i) << "] "
            "[" << name.at(i) << "] "
            "[" << language.at(i) << "]";

        if (type == "video") {
            ss1 << " [" << width.at(i) << height.at(i);

            /* get fps value */
            std::string s1 = duration.at(i);
            size_t l1 = s1.size();
            const std::string s2 = " frames/fields per second for a video track)";
            const size_t l2 = 44;  /* s2.size() */

            if (l1 > l2 && s1.substr(l1 - l2) == s2) {
                s1.erase(l1 - l2, l2);  /* remove s2 from the end of s1 */
                size_t pos = s1.find_last_of('(');

                if (pos != std::string::npos) {
                    s1.erase(0, pos + 1);

                    if (s1.find_first_not_of("0123456789.") == std::string::npos &&  /* only numbers and dots */
                        s1.find('.') == s1.rfind('.'))  /* no more than 1 dot */
                    {
                        /* strip trailing zeros */
                        ss1 << ", " << atof(s1.c_str()) << " fps";
                    }
                }
            }

            ss1 << "]";

            /* for now only extract timestamps of video streams */
            timestampIDs.push_back(i);
        }
        else if (type == "audio") {
            ss1 << " [" << channels.at(i) << freq.at(i) << "]";
        }

        trackInfos.push_back(ss1.str());

        ss2 << "track_" << i+1 << "_" << type;

        /* append the correct file extension
         * depending on the codec ID */
        for (const auto &e : mkv_codec_list) {
            if (codecid.at(i) == e.id) {
                ss2 << '.' << e.ext;
                break;
            }
        }

        trackFilenames.push_back(ss2.str());
    }

    for (size_t i = 0; i < filename.size(); i++) {
        std::stringstream ss;

        ss  << "Attachment " << i+1
            << " [" << filename.at(i) << "] ["
            << mime.at(i) << "] ["
            << fdata.at(i) << "]";

          attachmentInfos.push_back(ss.str());
          attachmentFilenames.push_back(filename.at(i));
    }

    return true;
}

