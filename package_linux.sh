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
curl -s -L -o b2d.zip https://github.com/digarok/b2d/releases/download/v1.5/b2d-ubuntu-latest-v1.5.zip
unzip -o cadius.zip -d cadius
unzip -o b2d.zip -d b2d
echo "${ID}: COPY UTILITY BINARIES"
chmod +x b2d/b2d # @TODO: Fix me
cp b2d/b2d $PACKAGEDIR
cp cadius/cadius $PACKAGEDIR

GOARCH=$( [ "$(uname -m)" = "aarch64" ] && echo arm64 || echo amd64 )
curl -s -L -o image2shr https://github.com/digarok/image2shr/releases/download/v0.2.0/image2shr-linux-${GOARCH}
chmod +x image2shr
cp image2shr $PACKAGEDIR

echo "${ID}: INSTALL EXTRA DEV PACKAGES"
sudo apt-get install -y libxkbcommon-x11-0 libxcb-cursor0

echo "${ID}: RUN DEPLOY BUILD"
wget -nv https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod +x linuxdeployqt-continuous-x86_64.AppImage
./linuxdeployqt-continuous-x86_64.AppImage --appimage-extract-and-run $PACKAGEDIR/buckshot -appimage -bundle-non-qt-libs -verbose=2 -unsupported-allow-new-glibc

zip -r buckshot-linux.zip $PACKAGEDIR
