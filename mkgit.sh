#!/bin/sh

mkdir dist
unzip -T ../dist/\*.zip

for f in ../dist/*
do
	distfile=`basename $f`
	cp $f objconv.zip
	rm -rf instructions.pdf objconv-instructions.pdf objconv.exe source extras changelog.txt
	unzip -o $f
	[ -f source.zip ] && unzip -o -d source source.zip
	[ -f extras.zip ] && unzip -o -d extras extras.zip
	rm -f source.zip extras.zip
	mv 'C:\_Public\objconv\x64\Release\objconv.exe' objconv.exe || true
	chmod 644 source/* extras/* changelog.txt objconv-instructions.pdf
	chmod 755 objconv.exe source/build.sh
	git add .
	git commit -m "${distfile%.zip}" --date="`LC_ALL=C date -r $f`"
done
