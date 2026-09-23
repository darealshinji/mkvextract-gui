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

#include <sstream>
#include <string>
#include <vector>
#include "check_browser.hpp"
#include "mkvextract.hpp"


std::string MKVextract::create_extraction_command(bool extract)
{
    bool has_tracks = false, has_attach = false;
    std::string command, base, attach_dir;

    m_args.clear();

    if (extract) {
        m_args.push_back("mkvextract");
        m_args.push_back(m_file);
        m_args.push_back("--ui-language");
        m_args.push_back("en_US");
        m_args.push_back("--gui-mode");
    } else {
        command = "mkvextract " + quote_filename(m_file);
    }

    if (m_use_source_path->value() == true) {
        base = m_outdir_source + file_stem(m_file);
    } else {
        base = m_outdir_manual + file_stem(m_file);
    }

    /* tracks */
    for (size_t i = 0; i < m_track_count; i++) {
        if (!m_browser->checked(i+1)) {
            continue;
        }

        if (!has_tracks) {
            has_tracks = true;

            if (extract) {
                m_args.push_back("tracks");
            } else {
                command += " tracks";
            }
        }

        std::stringstream ss;
        ss << i << ":" << base << " - " << m_outnames.at(i);

        if (extract) {
            m_args.push_back(ss.str());
        } else {
            command += " " + quote_filename(ss.str());
        }
    }

    /* attachments */
    if (m_attach_count > 0) {
        attach_dir = base + " - Attachments/";

        for (size_t i = 0; i < m_attach_count; i++) {
            if (!m_browser->checked(i + m_track_count + 1)) {
                continue;
            }

            if (!has_attach) {
                has_attach = true;

                if (extract) {
                    m_args.push_back("attachments");
                } else {
                    command += " attachments";
                }
            }

            std::stringstream ss;
            ss << i+1 << ":" << attach_dir << m_outnames.at(i + m_track_count);

            if (extract) {
                m_args.push_back(ss.str());
            } else {
                command += " " + quote_filename(ss.str());
            }
        }
    }

    /* timestamps */
    if (m_timestamps_entry > 0 && m_browser->checked(m_timestamps_entry)) {
        if (extract) {
            m_args.push_back("timestamps_v2");
        } else {
            command += " timestamps_v2";
        }

        for (size_t i = 0; i < m_timestampIDs.size(); i++) {
            int id = m_timestampIDs.at(i);
            std::stringstream ss;
            ss << id << ":" << base << " - track_" << id+1 << "_video_timestamps_v2.txt";

            if (extract) {
                m_args.push_back(ss.str());
            } else {
                command += " " + quote_filename(ss.str());
            }
        }
    }

    /* chapters */
    if (m_chapters_entry > 0 && m_browser->checked(m_chapters_entry)) {
        std::string s = base + " - chapters.xml";

        if (extract) {
            m_args.push_back("chapters");
            m_args.push_back(s);
        } else {
            command += " chapters " + quote_filename(s);
        }
    }

    /* tags */
    if (m_tags_entry > 0 && m_browser->checked(m_tags_entry)) {
        std::string s = base + " - tags.xml";

        if (extract) {
            m_args.push_back("tags");
            m_args.push_back(s);
        } else {
            command += " tags " + quote_filename(s);
        }
    }

    return command;
}

