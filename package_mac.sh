PACKAGEDIR=buckshot-package-osx
APPBUILDDIR=buckshot/buckshot.app
CONTENTDIR=${PACKAGEDIR}/buckshot.app/Contents
ASSETDIR=assets

ID=${0##*/}

# SET PATH
echo "${ID}: SET QT ENV"
. qt-env.sh


echo "${ID}: COPY EXTRA FILES"
mkdir -p $CONTENTDIR/MacOS
mkdir -p $CONTENTDIR/Resources
cp $ASSETDIR/Info.plist $CONTENTDIR
cp $ASSETDIR/icons.icns $CONTENTDIR/Resources
cp README.md $PACKAGEDIR/README.txt
cp LICENSE.txt $PACKAGEDIR

echo "${ID}: RUN macdeployqt"
macdeployqt $APPBUILDDIR
cp -r $APPBUILDDIR $PACKAGEDIR

echo "${ID}: DOWNLOAD UTILITY BINARIES"
curl -L -o cadius.zip https://github.com/digarok/cadius/releases/download/0.0.0/cadius-macos-latest-0.0.0.zip
curl -L -o b2d.zip https://github.com/digarok/b2d/releases/download/v1.3/b2d-macos-latest-v1.3.zip
unzip cadius.zip -d cadius
unzip b2d.zip -d b2d

chmod +x b2d/b2d # @TODO: Fix me
cp b2d/b2d $CONTENTDIR/MacOS
cp cadius/cadius $CONTENTDIR/MacOS

echo "${ID}: CREATE DMG"
git clone https://github.com/andreyvit/yoursway-create-dmg.git
cd yoursway-create-dmg

test -f buckshot.dmg && rm buckshot.dmg
./create-dmg \
  --volname "buckshot" \
  --volicon "../assets/icons.icns" \
  --background "../assets/buckshot_dmg_bg_600x500.png" \
  --window-pos 200 120 \
  --window-size 600 500 \
  --icon-size 100 \
  --icon buckshot.app 180 130 \
  --hide-extension buckshot.app \
  --icon README.txt  200 330 \
  --icon LICENSE.txt  410 330 \
  --app-drop-link 410 130 \
  --skip-jenkins \
  buckshot.dmg \
  ../${PACKAGEDIR}/
cp buckshot.dmg ..

exit 0

#############################################
### NONE SHALLL OPPAAPSADFAPAPSFPSAPPGFPED
#############################################
SDIR=build-buckshot-Desktop_Qt_5_12_0_clang_64bit-Release/buckshot.app
DEXTRAS=buckshot-osx/
DDIR=buckshot-osx/buckshot.app



mkdir -p $DDIR
# make app bundle with qt frameworks using macdeployqt
~/Qt/5.12.0/clang_64/bin/macdeployqt $SDIR
# copy to dmg staging dir
cp -R $SDIR $DEXTRAS

# not needed?
#dylibbundler -od -b -x $DDIR/Contents/MacOS/buckshot -d $DDIR/Contents/libs/




# files to include in dmg
#cp doc/gsplusmanual.pdf $DEXTRAS
#cp doc/gsplusmanual.txt $DEXTRAS

# COPY BINARIES FROM EXTERNAL PROJECTS
