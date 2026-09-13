/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2026 djcj <djcj@gmx.de>
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

/** MKVToolnix XML format:
<Chapters>
  <EditionEntry>
    <EditionFlagHidden>0</EditionFlagHidden>
    <ChapterAtom>
      <ChapterTimeStart>00:00:00.000000000</ChapterTimeStart>
      <ChapterDisplay>
        <ChapterString>Chapter 1</ChapterString>
      </ChapterDisplay>
    </ChapterAtom>
  </EditionEntry>
</Chapters>
**/

/**
 * OGM format:
CHAPTER01=00:00:00.000
CHAPTER01NAME=Chapter 01
CHAPTER02=00:21:34.534
CHAPTER02NAME=Chapter 02
**/

#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <tinyxml2.h>
#include "mkvextract.hpp"


static inline bool empty(const char *str)
{
    return (str == NULL || *str == 0);
}


static inline const char *get_text(tinyxml2::XMLElement *elem, const char *label)
{
    auto element = elem->FirstChildElement(label);

    return element ? element->GetText() : NULL;
}


static const char *get_chapter_name(tinyxml2::XMLElement *atom)
{
    const char *name = NULL;

    for (auto disp = atom->FirstChildElement("ChapterDisplay");
         disp != NULL;
         disp = disp->NextSiblingElement("ChapterDisplay"))
    {
        const char *str = get_text(disp, "ChapterString");

        if (empty(str)) {
            continue;
        }

        /* set to the first string we find */
        if (!name) {
            name = str;
        }

        const char *lang = get_text(disp, "ChapterLanguage");

        /* prefer English entries */
        if (lang && strcmp("eng", lang) == 0) {
            return str;
        }
    }

    return name;
}


static void save_chapter_entry(tinyxml2::XMLElement *atom, std::string time, int i, std::string &ogm)
{
    /* use std::regex to read the time values */
    std::smatch sm;
    const std::regex reg("^([0-9]+):([0-9]+):([0-9]+\\.[0-9]+|[0-9]+)");

    if (!std::regex_match(time, sm, reg) || sm.size() != 4) {
        return;
    }

    int h = atoi(sm.str(1).c_str());   /* hours */
    int m = atoi(sm.str(2).c_str());   /* minutes */
    float s = atof(sm.str(3).c_str()); /* seconds + splitseconds */

    if (m >= 60 || s >= 60.0) {
        return;
    }

    /* save numbers to text with precision and width set */
    char buf_num[32], buf_time[128];
    snprintf(buf_num, sizeof(buf_num)-1, "%02d", i);
    snprintf(buf_time, sizeof(buf_time)-1, "%02d:%02d:%06.3f", h, m, s);

    /* OGM format chapter entry */
    std::stringstream strm;
    strm << "CHAPTER" << buf_num << '=' << buf_time << '\n'; /* time begin */
    strm << "CHAPTER" << buf_num << "NAME=";                 /* chapter name */

    /* chapter name */
    const char *name = get_chapter_name(atom);

    if (empty(name)) {
        strm << "Chapter " << buf_num << '\n';
    } else {
        strm << name << '\n';
    }

    /* save entry */
    ogm += strm.str();
}


static inline bool query_int(tinyxml2::XMLElement *elem, int &value)
{
    return (elem->QueryIntText(&value) == tinyxml2::XML_SUCCESS);
}


bool xml2ogm(const char *input, const char *output)
{
    std::string ogm;
    std::ofstream ofs;
    tinyxml2::XMLDocument doc;
    int val = 0;

    if (empty(input) || empty(output)) {
        return false;
    }

    /* load file and find "Chapters" element */

    if (doc.LoadFile(input) != tinyxml2::XML_SUCCESS) {
        return false;
    }

    auto elem = doc.FirstChildElement("Chapters");

    if (!elem) {
        return false;
    }

    /* EditionEntry */
    auto entry = elem->FirstChildElement("EditionEntry");

    for ( ; entry != NULL; entry = entry->NextSiblingElement("EditionEntry")) {
        elem = entry->FirstChildElement("EditionFlagHidden");

        if (!elem || (query_int(elem, val) && val == 0)) {
            /* entry not hidden */
            break;
        }
    }

    if (!entry) {
        return false;
    }

    /* ChapterAtom entries */
    auto atom = entry->FirstChildElement("ChapterAtom");

    for (int i = 1; atom != NULL; ++i, atom = atom->NextSiblingElement("ChapterAtom"))
    {
        elem = atom->FirstChildElement("ChapterFlagHidden");

        if (elem && query_int(elem, val) && val != 0) {
            /* entry hidden */
            continue;
        }

        elem = atom->FirstChildElement("ChapterFlagEnabled");

        if (elem && query_int(elem, val) && val == 0) {
            /* entry not enabled */
            continue;
        }

        elem = atom->FirstChildElement("ChapterTimeStart");

        if (!elem) {
            continue;
        }

        const char *time = elem->GetText();

        /* save OGM format chapter entry */
        if (!empty(time)) {
            save_chapter_entry(atom, time, i, ogm);
        }
    }

    if (ogm.empty()) {
        return false;
    }

    /* save text to file */
    ofs.open(output);

    if (!ofs.is_open()) {
        return false;
    }

    ofs.write(ogm.c_str(), ogm.length());

    return true;
}

