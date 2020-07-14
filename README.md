# AppImageToDeb

#### 介绍
使用DTK开发界面
可以很方便的将AppImage软件转换成deb安装包。

#### 使用说明

1.  点击按钮打开Appimage文件或者拖入
2.  自动读取部分信息后检查和补充完整有关信息
3.  点击打包即可开始打包
4.  将在Appimage文件相同目录下生成deb文件
5.  如果使用“解包后打包”可能导致软件爱呢无法运行，同时也会大幅度加长打包时间

#### 依赖说明

1. Dtk：
    - libdtkcore5
    - libdtkgui5
    - libdtkwidget5
    --若使用deepin 15.11 / ubuntu kylin 
    - libdtkcore2
    - libdtkgui2
    - libdtkwidget2
2. qt：
    - libqt5core5a
    - libqt5gui5
    - libqt5svg5
    - libqt5concurrent5
3. 其他：
    - fakeroot
4. 备注
    - 若源码编译，请使用-dev包，如libdtkcore2-dev
#### 参与贡献

1.  Fork 本仓库


