/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018, djcj <djcj@gmx.de>
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

#include <string>
#include <stdio.h>
#include <string.h>

void decode_uri(std::string &src)
{
  std::string dest;

  for (size_t i = 0; i < src.length(); i++) {
    if (src[i] == '%') {
      int n;
      if (sscanf(src.substr(i + 1, 2).c_str(), "%2X", &n) != 1) {
        break;
      }
      dest += static_cast<char>(n);
      i += 2;
    } else {
      dest += src[i];
    }
  }
  src = dest;
}

bool file_is_matroska(const char *file)
{
  FILE *fp;
  size_t len;
  unsigned char bytes[32];

  if (!file || !(fp = fopen(file, "r"))) {
    return false;
  }

  len = fread(bytes, 1, sizeof(bytes), fp);

  if (ferror(fp) || len < sizeof(bytes)) {
    fclose(fp);
    return false;
  }
  fclose(fp);

  /**
   * See /usr/share/mime/packages/freedesktop.org.xml
   * or https://cgit.freedesktop.org/xdg/shared-mime-info/tree/freedesktop.org.xml.in
   */
  if (memcmp(bytes, "\x1A\x45\xDF\xA3", 4) == 0 && memcmp(bytes+21, "\x42\x82", 2) == 0 &&
      (memcmp(bytes+24, "matroska", 8) == 0 || memcmp(bytes+24, "webm", 4) == 0))
  {
    return true;
  }
  return false;
}
