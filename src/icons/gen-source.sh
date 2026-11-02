#!/bin/sh -e

scour -i mkvextract.svg \
      -o mkvextract_opt.svg \
      --strip-xml-prolog \
      --remove-descriptive-elements \
      --enable-comment-stripping \
      --enable-id-stripping \
      --shorten-ids \
      --strip-xml-space

file="icons.i"
rm -f $file

for name in mkvextract_opt settings; do
    echo "static const unsigned char ${name}_data[] = {" >> $file
    xxd -i < ${name}.svg >> $file
    echo "};" >> $file
done

