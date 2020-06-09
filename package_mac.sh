PACKAGEDIR=buckshot-package-osx
APPBUILDDIR=buckshot/buckshot.app
CONTENTDIR=${PACKAGEDIR}/buckshot.app/Contents
ASSETDIR=assets

ID=${0##*/}

# SET PATH
#echo "${ID}: SET QT ENV"
#. qt-env.sh

echo "${ID}: RUN macdeployqt"
macdeployqt $APPBUILDDIR

echo "${ID}: MKDIR PACKAGEDIR"
mkdir -p $PACKAGEDIR

echo "${ID}: COPY APPBUILDDIR -> PACKAGEDIR"
cp -r $APPBUILDDIR $PACKAGEDIR

echo "${ID}: COPY EXTRA FILES"
mkdir -p $CONTENTDIR/MacOS
mkdir -p $CONTENTDIR/Resources
cp $ASSETDIR/Info.plist $CONTENTDIR 
cp $ASSETDIR/icons.icns $CONTENTDIR/Resources
cp README.md $PACKAGEDIR/README.txt
cp LICENSE.txt $PACKAGEDIR

echo "${ID}: DOWNLOAD UTILITY BINARIES"
curl -s -L -o cadius.zip https://github.com/digarok/cadius/releases/download/0.0.0/cadius-macos-latest-0.0.0.zip
curl -s -L -o b2d.zip https://github.com/digarok/b2d/releases/download/v1.4/b2d-macos-latest-v1.4.zip
unzip -o cadius.zip -d cadius
unzip -o b2d.zip -d b2d


chmod +x b2d/b2d # @TODO: Fix me
cp b2d/b2d $CONTENTDIR/MacOS
cp cadius/cadius $CONTENTDIR/MacOS

