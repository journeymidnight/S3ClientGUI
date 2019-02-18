# Ubuntu

## Build shared library

```
mkdir build
cd build
cmake -DBUILD_ONLY="s3;transfer" ..
make
make install
```

## Install QT

```
sudo apt-get install  qt5-base

```

# extra

+ check pkg-config
+ remove -Werror


# Mac

## Install xcode

+ Install xcode from appstore
+ install command tools
```
xcode-selector --install
```
OR
```
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
```

## Download QT From achive
http://download.qt.io/official_release


## Build shared library

```
mkdir build
cd build
cmake -DBUILD_ONLY="s3" ..
make
make install
```


## build

```
qmake ..
make
```

## deploy

1. add deps
macdeployqt S3Client.GUI -libpath=/usr/local/lib/
2. build dmg
http://stackoverflow.com/questions/37292756/how-to-create-a-dmg-file-for-a-app-for-mac


# Windows

## Download QT From achive
http://download.qt.io/official_release


## install Visual Studio 2015 community version 

## install Qt plugin for visual studio

## nuget to install aws-sdk-core/aws-sdk-s3

version 1.0.108

## open .pro file


