/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020, djcj <djcj@gmx.de>
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

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include "tinyxml2.h"


bool xml2ogm(const char *input, const char *output)
{
  std::string ogm;
  std::ofstream ofs;
  tinyxml2::XMLDocument xmlDoc;
  tinyxml2::XMLElement *p, *pEE, *pCA;
  int val;

  if (!input || !output || strlen(input) < 1 || strlen(output) < 1) {
    return false;
  }

  if (xmlDoc.LoadFile(input) != tinyxml2::XML_SUCCESS) {
    return false;
  }

  p = xmlDoc.FirstChildElement("Chapters");
  if (!p) {
    return false;
  }

  /* EditionEntry */

  pEE = p->FirstChildElement("EditionEntry");

  while (pEE) {
    val = 0;
    p = pEE->FirstChildElement("EditionFlagHidden");

    if (!p || p->QueryIntText(&val) != tinyxml2::XML_SUCCESS || val == 0) {
      break;
    }
    pEE = pEE->NextSiblingElement("EditionEntry");
  }

  if (!pEE) {
    return false;
  }

  /* ChapterAtom */

  pCA = pEE->FirstChildElement("ChapterAtom");
  if (!pCA) {
    return false;
  }

  for (int i = 1; pCA != nullptr && i < 100; ++i,
       pCA = pCA->NextSiblingElement("ChapterAtom"))
  {
    int h, m;
    float s;
    const char *label, *time, *fmt;
    char buf[64];
    tinyxml2::XMLElement *pCD;

    /* hidden flag */
    val = 0;
    p = pCA->FirstChildElement("ChapterFlagHidden");
    if (p && (p->QueryIntText(&val) != tinyxml2::XML_SUCCESS || val != 0)) {
      continue;
    }

    /* enabled flag */
    val = 1;
    p = pCA->FirstChildElement("ChapterFlagEnabled");
    if (p && (p->QueryIntText(&val) != tinyxml2::XML_SUCCESS || val != 1)) {
      continue;
    }

    /* chapter title */
    pCD = pCA->FirstChildElement("ChapterDisplay");

    while (pCD) {
      const char *lang = NULL, *text = NULL;

      p = pCD->FirstChildElement("ChapterString");
      if (p) {
        text = p->GetText();
      }

      p = pCD->FirstChildElement("ChapterLanguage");
      if (p) {
        lang = p->GetText();
      }

      if (text && !label) {
        label = text;
      }

      /* prefer English entries */
      if (text && lang && strcmp("eng", lang) == 0) {
        label = text;
        break;
      }

      pCD = pCD->NextSiblingElement("ChapterDisplay");
    }

    /* chapter time */
    p = pCA->FirstChildElement("ChapterTimeStart");
    if (!p) {
      return false;
    }

    time = p->GetText();
    if (!time) {
      return false;
    }

    if (sscanf(time, "%d:%d:%f", &h, &m, &s) != 3) {
      return false;
    }

    /* check time limits */
    if (h<0||h>99 || m<0||m>59 || s<0||s>=60) {
      return false;
    }

    /* append ogm entries */
    if (label) {
      fmt = "CHAPTER%02d=%02d:%02d:%06.3f\nCHAPTER%02dNAME=";
      snprintf(buf, sizeof(buf) - 1, fmt, i, h, m, s, i);
      ogm += buf;
      ogm += label;
      ogm.push_back('\n');
    } else {
      fmt = "CHAPTER%02d=%02d:%02d:%06.3f\nCHAPTER%02dNAME=Chapter %02d\n";
      snprintf(buf, sizeof(buf) - 1, fmt, i, h, m, s, i, i);
      ogm += buf;
    }
  }

  if (ogm.empty()) {
    return false;
  }

  ofs.open(output);

  if (!ofs.is_open()) {
    return false;
  }

  ofs.write(ogm.c_str(), ogm.length());
  ofs.close();

  return true;
}

