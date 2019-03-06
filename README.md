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

## Using aws-sdk-cpp
* nuget to install aws-sdk-core/aws-sdk-s3
  * current version `1.6.20060301.25` may cause a crash when ListObjects() was involed if an object starts with Chinese character
* OR build shared library manually (Recommended)

```
git clone https://github.com/aws/aws-sdk-cpp.git
git checkout 1.7.57
mkdir build
cd build
cmake .. -G "Visual Studio 15 Win64" -DCMAKE_BUILD_TYPE=Release -DBUILD_ONLY="s3" -DENABLE_TESTING=OFF
msbuild ALL_BUILD.vcxproj
msbuild INSTALL.vcxproj /p:Configuration=Release
```

## open .pro file
If build aws-sdk-cpp library manually, please add the following properties in the project in Visual Studio:

### [VC++ Directories]
#### Include Directories
```
C:\Program Files\aws-cpp-sdk-all\include
```
#### Library Directories
```
C:\Program Files\aws-cpp-sdk-all\bin
```

### [Linker >> Input]
#### Additional Library Directories
```
aws-cpp-sdk-core.lib
aws-cpp-sdk-s3.lib
```

### [C/C++ >> Preprocessor]
#### Preprocessor Definitions
```
USE_WINDOWS_DLL_SEMANTICS
USE_IMPORT_EXPORT
```