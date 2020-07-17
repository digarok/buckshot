PACKAGEDIR=buckshot-package-ubuntu
APP=buckshot/buckshot
CONTENTDIR=${PACKAGEDIR}/buckshot.app/Contents
ASSETDIR=assets

ID=${0##*/}

echo "${ID}: MKDIR PACKAGEDIR"
mkdir -p $PACKAGEDIR

echo "${ID}: COPY APP -> PACKAGEDIR"
cp -r $APP $PACKAGEDIR

echo "${ID}: COPY EXTRA COMMON FILES"
cp README.md $PACKAGEDIR/README.txt
cp LICENSE.txt $PACKAGEDIR
cp assets/buckshot.desktop $PACKAGEDIR
cp assets/icon256.png $PACKAGEDIR/buckshot.png
cp assets/examples/* $PACKAGEDIR/


echo "${ID}: DOWNLOAD UTILITY BINARIES"
curl -s -L -o cadius.zip https://github.com/digarok/cadius/releases/download/0.0.0/cadius-ubuntu-latest-0.0.0.zip
curl -s -L -o b2d.zip https://github.com/digarok/b2d/releases/download/v1.4/b2d-ubuntu-latest-v1.4.zip
unzip -o cadius.zip -d cadius
unzip -o b2d.zip -d b2d
echo "${ID}: COPY UTILITY BINARIES"
chmod +x b2d/b2d # @TODO: Fix me
cp b2d/b2d $PACKAGEDIR
cp cadius/cadius $PACKAGEDIR

echo "${ID}: INSTALL EXTRA DEV PACKAGES"
sudo apt-get install -y libxkbcommon-x11-0

echo "${ID}: RUN DEPLOY BUILD"
wget -nv https://github.com/probonopd/linuxdeployqt/releases/download/5/linuxdeployqt-5-x86_64.AppImage
chmod +x linuxdeployqt-5-x86_64.AppImage
./linuxdeployqt-5-x86_64.AppImage $PACKAGEDIR/buckshot -appimage -bundle-non-qt-libs -verbose=2

zip -r buckshot-linux.zip $PACKAGEDIR
