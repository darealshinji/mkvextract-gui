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

#include <stddef.h>
#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "check_browser.hpp"
#include "mkvextract.hpp"


void MKVextract::create_cmd(int val)
{
    bool has_tracks = false, has_attach = false;
    std::string base, attach_dir;

    auto add_option = [&, this] (const char *str) {
        if (val == CREATE_VECTOR) {
            m_args.emplace_back(str);
        } else {
            m_txtbuf->append(" ", 1);
            m_txtbuf->append(str);
        }
    };

    auto add_path = [&, this] (std::string str) {
        if (val == CREATE_VECTOR) {
            m_args.push_back(std::move(str));
        } else {
            std::string s = ' ' + utils::quote_filename(str);
            m_txtbuf->append(s.c_str(), s.length());
        }
    };


    m_args.clear();

    if (val == CREATE_VECTOR) {
        m_args = { "mkvextract", m_file, "--ui-language", "en_US", "--gui-mode" };
    } else {
        std::string s = "mkvextract " + utils::quote_filename(m_file);
        m_txtbuf->replace(0, m_txtbuf->length(), s.c_str(), s.length());
    }

    base = stem_path();

    /* tracks */
    for (size_t i = 0; i < m_track_count; i++) {
        if (!m_browser->checked(i+1)) {
            continue;
        }

        if (!has_tracks) {
            has_tracks = true;
            add_option("tracks");
        }

        std::stringstream strm;
        strm << i << ':' << base << " - " << m_outnames.at(i);
        add_path(strm.str());
    }

    /* attachments */
    if (m_attach_count > 0) {
        attach_dir = base + " - Attachments/";

        for (size_t i = 0; i < m_attach_count; i++) {
            if (!m_browser->checked(m_track_count + i + 1)) {
                continue;
            }

            if (!has_attach) {
                has_attach = true;
                add_option("attachments");
            }

            std::stringstream strm;
            strm << i+1 << ':' << attach_dir << m_outnames.at(m_track_count + i);
            add_path(strm.str());
        }
    }

    /* timestamps */
    if (m_timestamps_entry > 0 && m_browser->checked(m_timestamps_entry)) {
        add_option("timestamps_v2");

        for (size_t i = 0; i < m_timestampIDs.size(); i++) {
            int id = m_timestampIDs.at(i);
            std::stringstream strm;
            strm << id << ':' << base << " - track " << id+1 << " timestamps.txt";
            add_path(strm.str());
        }
    }

    /* chapters */
    if (m_chapters_entry > 0 && m_browser->checked(m_chapters_entry)) {
        add_option("chapters");
        add_path(base + " - chapters.xml");
    }

    /* tags */
    if (m_tags_entry > 0 && m_browser->checked(m_tags_entry)) {
        add_option("tags");
        add_path(base + " - tags.xml");
    }
}

