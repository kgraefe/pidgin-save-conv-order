#!/bin/bash
set -x

ls -1 src/*.c > po/POTFILES.in

cd po
intltool-update -po

for f in *.po
do test -f $f && intltool-update ${f%.po}
done

