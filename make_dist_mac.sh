DEXTRAS=buckshot-osx/
DDIR=buckshot-osx/buckshot.app
ADIR=assets

mkdir -p $DDIR
# make app bundle with qt frameworks using macdeployqt
~/Qt/5.7/clang_64/bin/macdeployqt build-buckshot-Desktop_Qt_5_7_0_clang_64bit-Release/buckshot.app
# copy to dmg staging dir
cp -R build-buckshot-Desktop_Qt_5_7_0_clang_64bit-Release/buckshot.app $DEXTRAS

mkdir -p $DDIR/Contents/MacOS
mkdir -p $DDIR/Contents/Resources
cp $ADIR/Info.plist $DDIR/Contents
cp $ADIR/icons.icns $DDIR/Contents/Resources
# not needed?
#dylibbundler -od -b -x $DDIR/Contents/MacOS/buckshot -d $DDIR/Contents/libs/




# files to include in dmg
#cp doc/gsplusmanual.pdf $DEXTRAS
#cp doc/gsplusmanual.txt $DEXTRAS
cp README.txt $DEXTRAS
cp LICENSE.txt $DEXTRAS

# COPY BINARIES FROM EXTERNAL PROJECTS 
cp ../b2d $DDIR/Contents/MacOS
cp ../tools/Cadius $DDIR/Contents/MacOS

# packaging now in DMG script
