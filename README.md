# buckshot
An image conversion tool for Mac OSX, Windows and Linux.  Take modern image formats (PNG, JPEG, BMP, etc) and convert them for use in your Apple II programs or just for fun.

![Screenshot of starting the program](doc/web/Screenshot.png "Screenshot of starting the program")

# Installation and Launching
The fastest way to get started is to go here [https://apple2.gs/buckshot](https://apple2.gs/buckshot) where you can download the latest builds for Mac OSX, Windows and Linux.

### Mac OSX
Download and open the DMG file.  Drag buckshot into your applications and launch it from there.  
NOTE! Recent MacOS versions require you to open the "Applications" folder in Finder and right-click -> Choose "Open", as a ~~gatekeeping~~ security precaution.)

### Windows
Download and unzip to the folder where you want it installed.  Simply run the buckshot.exe from there. 
Note: Windows may warn and require you to "run anyway".

### Linux (Debian)
Download and unzip and run `AppRun`.  If you have problems see the README file. 

# Usage

Once you start the program, just "Open Source Image", select the "Apple ][ Display Mode" you want to convert your image to, and click "Preview", or even better, click the "Live Preview" checkbox to get real-time feedback on your conversion settings.  Once you are satisfied with your conversion settings, click "Save Image File" to save in one of the Apple ][ image formats based on the display mode.  If you want to save that image file directly to a ProDOS volume, that is now supported via the "Save To ProDOS" function!  Then you can load it up in your favorite emulator, or transfer it to real disks/flash storage to view on glorious vintage hardware.

# Build
This project is written in C++ using the Qt framework.  It calls out to two external binaries for image conversion ([b2d](http://www.appleoldies.ca/bmp2dhr/)) and ProDOS volume support ([CADIUS](http://www.brutaldeluxe.fr/products/crossdevtools/cadius/)).

You can build and run the project without those them, but it won't be able to generate previews or save to ProDOS volumes without those two 3rd-party binaries.  The authors of those projects are not involved with this project, but have graciously encouraged my integration attempts with this software.

I maintain public builds of both utilities for all supported operating systems available here:

- https://github.com/digarok/cadius/releases/
- https://github.com/digarok/b2d/releases/

You will need to copy the binaries of those two programs for your platform into the build directory you are running.

Example for Mac OSX "Release" build:
```
cp ../b2d build-buckshot-Desktop_Qt_5_7_0_clang_64bit-Release/buckshot.app/Contents/MacOS/
cp ../Cadius build-buckshot-Desktop_Qt_5_7_0_clang_64bit-Release/buckshot.app/Contents/MacOS/
```
In this example I obviously have the files stored in a parent directory outside of the project folder.

The binaries for those can also just be pulled out of existing downloads available on the [release page](https://github.com/digarok/buckshot/releases/).

# Disclaimer
This was largely built as a proof-of-concept, and I tried to put it together quite quickly (the original version was over the course of about a week or two.)  I would never suggest that this is ididiomatic C++ code or Qt code.  I am very results oriented and wrote a lot of this quite procedurally, as I went along adding all my planned features.  Please feel free to fork it and make fixes, clean it up, add in other converters, etc.  I probably won't have much time to tinker with this one from here on out, as I have most of the features I need for my own usage at this point. 


# Credits
I cannot begin to express my gratitude to the excellent programmers who authored the tools doing the heavy lifting under-the-hood of buckshot.  

### bmp2dhr
Thanks to Bill Buckels, whose seminal work in image conversion routines has long inspired me to improve my own IIgs conversion routines. 

### CADIUS
Thanks to Brutal Deluxe, whose tools not only speed up my entire Apple II development pipeline, but also provides the ProDOS image support here.



