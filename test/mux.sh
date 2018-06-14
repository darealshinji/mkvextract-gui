#!/bin/sh

cd "$(dirname "$0")"

out="test.mkv"

mkvmerge \
  --output "$out" \
  --title 'Matroska Example' \
    \
  --language '0:zxx' --track-name '0:test pattern' video.hevc \
  --language '0:eng' --track-name '0:static 1' static1.flac \
  --language '0:und' --track-name '0:static 2' static2.ogg \
  --language '0:eng' --track-name '0:English (ASS)' english.ass \
  --language '0:eng' --track-name '0:English (SRT)' english.srt \
  --language '0:ger' --track-name '0:Deutsch (ASS)' german.ass \
  --language '0:ger' --track-name '0:Deutsch (SRT)' german.srt \
  \
  --attachment-name test-pattern.y4m.xz \
  --attachment-mime-type 'application/x-xz' \
  --attach-file test-pattern.y4m.xz \
  \
  --attachment-name legalcode.txt \
  --attachment-mime-type 'text/plain' \
  --attach-file legalcode.txt \
  \
  --chapter-language eng --chapters chapters.xml

if [ $? -eq 0 ]; then
  echo "\`$PWD/$out' successfully created"
fi

