#!/bin/bash

VERSION=0.2
BUILDDIR=~
DISTDIR=$BUILDDIR/buckshot-dist-linux
LIBDIR=$DISTDIR/lib
PLATFORMDIR=$DISTDIR/platforms


rm -rf $DISTDIR
mkdir -p $DISTDIR
mkdir -p $LIBDIR
mkdir -p $PLATFORMDIR

cp build-buckshot-Desktop_Qt_5_7_0_GCC_64bit-Release/buckshot $DISTDIR
cp build-buckshot-Desktop_Qt_5_7_0_GCC_64bit-Release/b2d $DISTDIR
cp build-buckshot-Desktop_Qt_5_7_0_GCC_64bit-Release/Cadius $DISTDIR
cp platform/linux/buckshot.sh $DISTDIR
cp doc/README.LINUX $DISTDIR
cp README.md $DISTDIR

# As reported by ldd
cp /home/builder/Qt/5.7/gcc_64/lib/libicudata.so.56 $LIBDIR
cp /home/builder/Qt/5.7/gcc_64/lib/libicui18n.so.56 $LIBDIR
cp /home/builder/Qt/5.7/gcc_64/lib/libicuuc.so.56 $LIBDIR
cp /home/builder/Qt/5.7/gcc_64/lib/libQt5Core.so.5 $LIBDIR
cp /home/builder/Qt/5.7/gcc_64/lib/libQt5DBus.so.5 $LIBDIR
cp /home/builder/Qt/5.7/gcc_64/lib/libQt5Gui.so.5 $LIBDIR
cp /home/builder/Qt/5.7/gcc_64/lib/libQt5Widgets.so.5 $LIBDIR
cp /home/builder/Qt/5.7/gcc_64/lib/libQt5XcbQpa.so.5 $LIBDIR
cp /home/builder/Qt/5.7/gcc_64/plugins/platforms/libqxcb.so $PLATFORMDIR
cp -r /home/builder/Qt/5.7/gcc_64/plugins/imageformats $DISTDIR

tar -cvjf buckshot-dist-linux_$VERSION.tar.bz2 -C $BUILDDIR `basename $DISTDIR`
