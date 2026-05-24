#!/bin/bash

set -e

rm -rf deploy
mkdir -p deploybuild

pushd deploybuild
cmake .. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -GNinja -DCMAKE_BUILD_TYPE=Release
ninja
popd

mkdir deploy

cp ./deploybuild/videomaker.exe ./deploy/

cp /ucrt64/bin/libKF6WindowSystem.dll ./deploy/
cp /ucrt64/bin/libKirigamiPlatform.dll ./deploy/
mkdir ./deploy/iconengines
cp /ucrt64/share/qt6/plugins/kiconthemes6/iconengines/KIconEnginePlugin.dll ./deploy/iconengines/

ldd deploy/* |grep -iv system32|grep -vi windows|grep -v :$  | cut -f2 -d'>' | cut -f1 -d\( | tr '\' '/' |while read a; do ! [ -e "deploy/`basename $a`" ] && cp -v "$a" deploy/; done

pushd deploy
windeployqt6 videomaker.exe
popd

