#!/bin/bash

BUILDDIR=~/buckshot-dist-linux
LIBDIR=$BUILDDIR/lib
PLUGINDIR=$BUILDDIR/plugins
PLATFORMDIR=$BUILDDIR/platforms


rm -rf $BUILDDIR
mkdir -p $BUILDDIR
mkdir -p $LIBDIR
mkdir -p $PLUGINDIR
mkdir -p $PLATFORMDIR

cp build-buckshot-Desktop_Qt_5_7_0_GCC_64bit-Release/buckshot $BUILDDIR
cp build-buckshot-Desktop_Qt_5_7_0_GCC_64bit-Release/b2d $BUILDDIR
cp platform/linux/buckshot.sh $BUILDDIR

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
cp -r /home/builder/Qt/5.7/gcc_64/plugins/imageformats $PLUGINDIR

