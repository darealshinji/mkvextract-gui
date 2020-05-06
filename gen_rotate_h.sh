#!/bin/sh

sed -e 's,#0,@555,g; s,#1,@999,g; s,#2,@ddd,g; s,#3,@000,g; s,#4,@000,g; s,#5,@000,g; s,#6,@000,g; s,#7,@000,g; s,@,#,g' rotate.svg.in > 0.svg
sed -e 's,#0,@000,g; s,#1,@555,g; s,#2,@999,g; s,#3,@ddd,g; s,#4,@000,g; s,#5,@000,g; s,#6,@000,g; s,#7,@000,g; s,@,#,g' rotate.svg.in > 1.svg
sed -e 's,#0,@000,g; s,#1,@000,g; s,#2,@555,g; s,#3,@999,g; s,#4,@ddd,g; s,#5,@000,g; s,#6,@000,g; s,#7,@000,g; s,@,#,g' rotate.svg.in > 2.svg
sed -e 's,#0,@000,g; s,#1,@000,g; s,#2,@000,g; s,#3,@555,g; s,#4,@999,g; s,#5,@ddd,g; s,#6,@000,g; s,#7,@000,g; s,@,#,g' rotate.svg.in > 3.svg
sed -e 's,#0,@000,g; s,#1,@000,g; s,#2,@000,g; s,#3,@000,g; s,#4,@555,g; s,#5,@999,g; s,#6,@ddd,g; s,#7,@000,g; s,@,#,g' rotate.svg.in > 4.svg
sed -e 's,#0,@000,g; s,#1,@000,g; s,#2,@000,g; s,#3,@000,g; s,#4,@000,g; s,#5,@555,g; s,#6,@999,g; s,#7,@ddd,g; s,@,#,g' rotate.svg.in > 5.svg
sed -e 's,#0,@ddd,g; s,#1,@000,g; s,#2,@000,g; s,#3,@000,g; s,#4,@000,g; s,#5,@000,g; s,#6,@555,g; s,#7,@999,g; s,@,#,g' rotate.svg.in > 6.svg
sed -e 's,#0,@999,g; s,#1,@ddd,g; s,#2,@000,g; s,#3,@000,g; s,#4,@000,g; s,#5,@000,g; s,#6,@000,g; s,#7,@555,g; s,@,#,g' rotate.svg.in > 7.svg

rm -f rotate.h

but_h=28

for n in 0 1 2 3 4 5 6 7 ; do
  rsvg-convert -w $but_h -h $but_h -o ${n}_.png $n.svg
  pngfix --optimize --strip=all --out=$n.png ${n}_.png
  xxd -i $n.png ${n}_png.h
  cat ${n}_png.h >> rotate.h
  rm $n.png $n.svg ${n}_png.h ${n}_.png
done

