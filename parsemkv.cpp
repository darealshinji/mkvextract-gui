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

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>

static const char *codecList[][2] = {
  { "V_MS/VFW/FOURCC", "vfw" },
  { "V_UNCOMPRESSED", "raw" },
  { "V_MPEG4/ISO/AVC", "avc" },
  { "V_MPEG4/ISO/SP", "m4v" },
  { "V_MPEG4/ISO/ASP", "m4v" },
  { "V_MPEG4/ISO/AP", "m4v" },
  { "V_MPEG4/MS/V3", "m4v" },
  { "V_MPEG1", "m1v" },
  { "V_MPEG2", "m2v" },
  { "V_REAL/RV10", "rv" },
  { "V_REAL/RV20", "rv" },
  { "V_REAL/RV30", "rv" },
  { "V_REAL/RV40", "rv" },
  { "V_QUICKTIME", "qt" },
  { "V_THEORA", "ogv" },
  { "V_PRORES", "prores" },
  { "V_SNOW", "snow" },
  { "V_VP8", "vp8" },
  { "V_VP9", "vp9" },
  { "V_DIRAC", "dirac" },
  { "V_MPEGH/ISO/HEVC", "hevc" },
  { "A_MPEG/L1", "mp1" },
  { "A_MPEG/L2", "mp2" },
  { "A_MPEG/L3", "mp3" },
  { "A_PCM/INT/BIG", "wav" },
  { "A_PCM/INT/LIT", "wav" },
  { "A_PCM/FLOAT/IEEE", "wav" },
  { "A_MPC", "mpc" },
  { "A_AC3", "ac3" },
  { "A_AC3/BSID9", "ac3" },
  { "A_AC3/BSID10", "ac3" },
  { "A_EAC3", "ac3" },
  { "A_ALAC", "alac" },
  { "A_DTS", "dts" },
  { "A_DTS/EXPRESS", "dts" },
  { "A_DTS/LOSSLESS", "dts" },
  { "A_VORBIS", "ogg" },
  { "A_FLAC", "flac" },
  { "A_REAL/14_4", "ra" },
  { "A_REAL/28_8", "ra" },
  { "A_REAL/COOK", "ra" },
  { "A_REAL/SIPR", "ra" },
  { "A_REAL/RALF", "ra" },
  { "A_REAL/ATRC", "ra" },
  { "A_MS/ACM", "acm" },
  { "A_AAC", "aac" },
  { "A_AAC/MPEG2/MAIN", "aac" },
  { "A_AAC/MPEG2/LC", "aac" },
  { "A_AAC/MPEG2/LC/SBR", "aac" },
  { "A_AAC/MPEG2/SSR", "aac" },
  { "A_AAC/MPEG4/MAIN", "aac" },
  { "A_AAC/MPEG4/LC", "aac" },
  { "A_AAC/MPEG4/LC/SBR", "aac" },
  { "A_AAC/MPEG4/SSR", "aac" },
  { "A_AAC/MPEG4/LTP", "aac" },
  { "A_QUICKTIME", "qta" },
  { "A_QUICKTIME/QDMC", "qdmc" },
  { "A_OPUS", "opus" },
  { "A_MLP", "mlp" },
  { "A_TTA1", "tta" },
  { "A_WAVPACK4", "wv" },
  { "A_TRUEHD", "thd" },
  { "S_TEXT/UTF8", "srt" },
  { "S_TEXT/SSA", "ssa" },
  { "S_TEXT/ASS", "ass" },
  { "S_TEXT/USF", "usf" },
  { "S_TEXT/ASCII", "txt" },
  { "S_IMAGE/BMP", "bmp" },
  { "S_VOBSUB", "sub" },
  { "S_VOBSUB/ZLIB", "sub" },
  { "S_KATE", "kate" },
  { "S_HDMV/PGS", "pgs" },
  { NULL, NULL }
};

static const char *get_extension(std::string codecid)
{
  for (size_t i = 0; codecList[i][0] != NULL; i++) {
    if (codecList[i][0] == codecid) {
      return codecList[i][1];
    }
  }
  return NULL;
}

inline static bool checkLine(std::string &line, const std::string str)
{
  if (line.substr(0, str.size()) == str) {
    line.erase(0, str.size());
    return true;
  }
  return false;
}

inline static void pop_push_back(std::vector<std::string> &v, std::string s) {
  v.pop_back();
  v.push_back(s);
}

bool parsemkv(std::string file
,             std::vector<std::string> &trackInfos
,             std::vector<std::string> &trackFilenames
,             std::vector<std::string> &attachmentInfos
,             std::vector<std::string> &attachmentFilenames
,             std::vector<int> &timestampIDs
,             bool &has_chapters
,             std::string &error)
{
  std::ifstream ifs;
  std::vector<std::string> codecid, name, language, width, height, freq,
    channels, filename, mime, fdata;
  std::string command, line;

  error = "";

  char stats[] = "/tmp/mkvinfo-stats-XXXXXX";
  if (mkstemp(stats) == -1) {
    error = "cannot create temporary file:\n";
    error.append(stats);
    return false;
  }

  command = "mkvinfo --ui-language en_US -q \"" + file + "\" > ";
  command.append(stats);

  if (system(command.c_str()) != 0) {
    error = "mkvinfo has returned an error";
    return false;
  }

  ifs.open(stats, std::ifstream::in);
  if (!ifs.is_open()) {
    error = "cannot open temporary file:\n";
    error.append(stats);
    return false;
  }

  /* start parsing the stats file */

  std::getline(ifs, line);
  if (line != "+ EBML head") {
    error = "malformed stats file:\n";
    error.append(stats);
    ifs.close();
    return false;
  }

  const std::string
    S_codecid =  "|  + Codec ID: ",
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

  bool tracks_begin = false;
  const unsigned short tnone = 0, tvideo = 1, taudio = 2;
  unsigned short track_entry = tnone;
  has_chapters = false;

  /* parse tracks */
  while (std::getline(ifs, line)) {
    if (tracks_begin) {
      if (line == "| + Track") {
        codecid.push_back("");
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
        pop_push_back(codecid, line);
        continue;
      }
      else if (checkLine(line, S_name)) {
        pop_push_back(name, line);
        continue;
      }
      else if (checkLine(line, S_language)) {
        pop_push_back(language, line);
        continue;
      }
      else if (line[0] == '|' && line[1] == '+') {
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
          pop_push_back(width, line);
          continue;
        }
      else if (checkLine(line, S_height)) {
          line = "x" + line;
          pop_push_back(height, line);
          continue;
        }
      }
      else if (track_entry == taudio) {
        if (checkLine(line, S_channels)) {
          line += " channels";
          pop_push_back(channels, line);
          continue;
        }
        else if (checkLine(line, S_freq)) {
          line = ", " + line + "Hz";
          pop_push_back(freq, line);
          continue;
        }
        /* no entry means mono */
        pop_push_back(channels, "1 channel");
      }
    } else if (line == "|+ Tracks") {
      tracks_begin = true;
      continue;
    }
  }

  /* parse attachments and chapters */
  while (std::getline(ifs, line)) {
    if (line == "| + Attached") {
      filename.push_back("");
      mime.push_back("");
      fdata.push_back("");
      continue;
    }
    else if (checkLine(line, S_filename)) {
      pop_push_back(filename, line);
      continue;
    }
    else if (checkLine(line, S_mime)) {
      pop_push_back(mime, line);
      continue;
    }
    else if (checkLine(line, S_fdata)) {
      line += " bytes";
      pop_push_back(fdata, line);
      continue;
    }
    else if (line == "|+ Chapters") {
      has_chapters = true;
      break;
    }
  }

  ifs.close();
  unlink(stats);

  for (size_t i = 0; i < codecid.size(); i++) {
    std::stringstream ss1, ss2;
    const char *ext;
    std::string type;

    switch (codecid.at(i)[0]) {
      case 'V': type = "video"; break;
      case 'A': type = "audio"; break;
      case 'S': type = "subtitles"; break;
      default:  type = "unknown"; break;
    }

    ss1 << "Track " << i+1
      << " [TID " << i << "] ["
      << type << "] ["
      << codecid.at(i) << "] ["
      << name.at(i) << "] ["
      << language.at(i) << "]";

    if (type == "video") {
      ss1 << " ["
        << width.at(i)
        << height.at(i) << "]";

      timestampIDs.push_back(i);
    }
    else if (type == "audio") {
      ss1 << " ["
        << channels.at(i)
        << freq.at(i) << "]";
    }

    trackInfos.push_back(ss1.str());

    ss2 << "track_" << i+1 << "_" << type;
    if ((ext = get_extension(codecid.at(i))) != NULL) {
      ss2 << "." << ext;
    }
    trackFilenames.push_back(ss2.str());
  }

  for (size_t i = 0; i < filename.size(); i++) {
    std::stringstream ss;
    ss << "Attachment " << i+1
      << " [" << filename.at(i) << "] ["
      << mime.at(i) << "] ["
      << fdata.at(i) << "]";
    attachmentInfos.push_back(ss.str());
    attachmentFilenames.push_back(filename.at(i));
  }

  return true;
}

