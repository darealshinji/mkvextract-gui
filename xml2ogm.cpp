/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019, djcj <djcj@gmx.de>
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

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

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


static bool read_file(const char *file, std::string &str)
{
  char *buf;
  size_t len;
  std::ifstream fs;
  str = "";

  fs.open(file);

  if (!fs.is_open()) {
    return false;
  }

  fs.seekg(0, fs.end);
  len = fs.tellg();

  if (len > 1024*1024) {
    len = 1024*1024;
  }
  fs.seekg(0, fs.beg);

  buf = new char[len];
  fs.read(buf, len);
  fs.close();

  buf[len] = '\0';
  str = std::string(buf);
  delete[] buf;

  return true;
}

static std::string get_content(const std::string element, const std::string str, size_t *pos)
{
  size_t beg = str.find("<" + element + ">");
  size_t end = str.find("</" + element + ">");

  if (beg == std::string::npos || end == std::string::npos || beg > end) {
    *pos = 0;
    return "";
  }

  *pos = end + element.length() + 3;
  beg += element.length() + 2;

  return str.substr(beg, end - beg);
}

bool xml2ogm(const char *input, const char *output)
{
  size_t pos = 0;
  std::string xml;
  std::stringstream ogm;
  std::ofstream fs;

  if (!input || !output || strlen(input) < 1 || strlen(output) < 1) {
    return false;
  }

  if (!read_file(input, xml)) {
    return false;
  }

  if ((xml = get_content("Chapters", xml, &pos)) == "" ||
      (xml = get_content("EditionEntry", xml, &pos)) == "")
  {
    return false;
  }

  for (int n = 1; n < 100; n++) {
    std::string atom, str;
    std::stringstream ss;

    if ((atom = get_content("ChapterAtom", xml, &pos)) == "") {
      break;
    }

    xml.erase(0, pos);

    if ((str = get_content("ChapterTimeStart", atom, &pos)) == "") {
      break;
    }

    ss << "CHAPTER" << std::setw(2) << std::setfill('0') << n;
    ogm << ss.str() << "=" << str.substr(0, 12) << std::endl << ss.str() << "NAME=";

    if ((str = get_content("ChapterDisplay", atom, &pos)) != "" && (str = get_content("ChapterString", str, &pos)) != "") {
      ogm << str << std::endl;
    } else {
      ogm << "Chapter " << n << std::endl;
    }
  }

  if (ogm.str().length() < 1) {
    return false;
  }

  fs.open(output);

  if (!fs.is_open()) {
    return false;
  }

  fs.write(ogm.str().c_str(), ogm.str().length());
  fs.close();

  return true;
}
