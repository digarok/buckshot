DEXTRAS=buckshot-osx/
DDIR=buckshot-osx/buckshot.app
ADIR=assets

mkdir -p $DDIR
cp -R build-buckshot-Desktop_Qt_5_7_0_clang_64bit-Release/buckshot.app $DEXTRAS

mkdir -p $DDIR/Contents/MacOS
mkdir -p $DDIR/Contents/Resources
cp $ADIR/Info.plist $DDIR/Contents
cp $ADIR/icons.icns $DDIR/Contents/Resources
dylibbundler -od -b -x $DDIR/Contents/MacOS/buckshot -d $DDIR/Contents/libs/

# files to include in dmg
#cp doc/gsplusmanual.pdf $DEXTRAS
#cp doc/gsplusmanual.txt $DEXTRAS
cp doc/README.txt $DEXTRAS
cp ../b2d $DDIR/Contents/MacOS
#cp COPYING.txt $DEXTRAS

#tar -cvzf gsplus-osx.tar.gz gsplus-osx/

# packaging now in DMG script
