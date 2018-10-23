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
#include <vector>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void decode_uri(std::string &src)
{
  std::string dest;
  int n;

  for (size_t i = 0; i < src.length(); i++) {
    if (src[i] == '%') {
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

/* replace all instances of »'« with »'\''« and put
 * single quotes around the string */
void quote_filename(std::string &s)
{
  size_t pos;
  const size_t newlen = 4;  /* strlen("'\\''") == 4 */

  for (pos = 0; (pos = s.find("'", pos)) != std::string::npos; pos += newlen) {
    s.replace(pos, 1, "'\\''");
  }
  s.insert(0, 1, '\'');
  s.push_back('\'');
}

bool file_is_matroska(const char *file)
{
  FILE *fp;
  size_t len;
  unsigned char bytes[32] = {0};

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

FILE *popen_mkvextract(const std::vector<std::string> args, pid_t &pid)
{
  enum { r = 0, w = 1 };
  int fd[2];
  size_t len;
  char **child_argv;

  if (pipe(fd) == -1) {
    return NULL;
  }

  if ((pid = fork()) != 0) {
    close(fd[w]);
    return fdopen(fd[r], "r");
  }

  len = args.size();
  child_argv = new char *[len + 1];

  for (size_t i = 0; i < len; i++) {
    child_argv[i] = const_cast<char *>(args.at(i).c_str());
  }
  child_argv[len] = NULL;

  close(fd[r]);
  dup2(fd[w], 1);
  close(fd[w]);
  execvp("mkvextract", child_argv);

  delete[] child_argv;
  _exit(127);

  return NULL;
}

int run_mkvinfo(const char *infile, const char *logfile)
{
  int status;
  int rv = 127;
  pid_t id = fork();

  if (id == 0) {
    execlp("mkvinfo", "mkvinfo", "--ui-language", "en_US", "--redirect-output", logfile, infile, NULL);
    _exit(127);
  }

  if (id > 0 && waitpid(id, &status, 0) > 0 && WIFEXITED(status) == 1) {
    rv = WEXITSTATUS(status);
  }

  return rv;
}

