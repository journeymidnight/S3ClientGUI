# S3ClientGUI打包发布

## 得到S3ClientGUI.exe

用vs17生成S3ClientGUI的release版本，得到S3ClientGUI.exe，放到一个新的文件夹

## 使用windeployqt

从开始菜单找到"Qt for Desktop"的命令交互,进入上面有S3ClientGUI.exe的新文件夹路径，输入命令

```
>windeployqt S3ClientGUI.exe
```

这时候文件夹中会生成依赖文件

## 加入AWS库文件

需要在文件夹加入如下的AWS库文件的release版

```
aws-c-common.dll
aws-c-event-stream.dll
aws-checksums.dll
aws-cpp-sdk-core.dll
aws-cpp-sdk-s3.dll
```

## 再加入俩文件

从路径C:\Windows\System32找到下面文件加入上面的文件夹中
```
ucrtbase.dll
vcruntime140d.dll
```

这时S3ClientGUI.exe运行所需要的依赖基本全了