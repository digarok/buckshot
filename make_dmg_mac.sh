#!/bin/sh
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
  --icon README.txt  80 330 \
  --app-drop-link 410 130 \
  buckshot.dmg \
  ../buckshot-osx/
cp buckshot.dmg ..
  #--icon gsplusmanual.pdf  220 330 \
  #--icon gsplusmanual.txt  360 330 \
  #--icon COPYING.txt  500 330 \
