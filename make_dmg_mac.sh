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
  --icon README.txt  200 330 \
  --icon LICENSE.txt  410 330 \
  --app-drop-link 410 130 \
  buckshot.dmg \
  ../buckshot-osx/
cp buckshot.dmg ..
