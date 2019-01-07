
SDIR=build-buckshot-Desktop_Qt_5_12_0_clang_64bit-Release/buckshot.app
DEXTRAS=buckshot-osx/
DDIR=buckshot-osx/buckshot.app
ADIR=assets



mkdir -p $DDIR
# make app bundle with qt frameworks using macdeployqt
~/Qt/5.12.0/clang_64/bin/macdeployqt $SDIR
# copy to dmg staging dir
cp -R $SDIR $DEXTRAS

mkdir -p $DDIR/Contents/MacOS
mkdir -p $DDIR/Contents/Resources
cp $ADIR/Info.plist $DDIR/Contents
cp $ADIR/icons.icns $DDIR/Contents/Resources
# not needed?
#dylibbundler -od -b -x $DDIR/Contents/MacOS/buckshot -d $DDIR/Contents/libs/




# files to include in dmg
#cp doc/gsplusmanual.pdf $DEXTRAS
#cp doc/gsplusmanual.txt $DEXTRAS
cp README.md $DEXTRAS/README.txt
cp LICENSE.txt $DEXTRAS

# COPY BINARIES FROM EXTERNAL PROJECTS 
cp ../b2d $DDIR/Contents/MacOS
cp ../tools/Cadius $DDIR/Contents/MacOS

# packaging now in DMG script
