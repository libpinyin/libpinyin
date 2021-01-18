# libpinyin

Library to deal with pinyin.

The libpinyin project aims to provide the algorithms core for intelligent sentence-based Chinese pinyin input methods.

## 编译安装

发行版中自带版本较老, 如果需要安装新版可以从源码编译. 请从[发布页面](https://github.com/libpinyin/libpinyin/releases)选择一个版本下载并解压, 在源码文件夹中打开终端执行命令:
> 中国内地用户请提前科学上网, 下载编译需要资源[model19.text.tar.gz](http://downloads.sourceforge.net/libpinyin/models/model19.text.tar.gz). 将其放到源码文件夹的data目录中, 然后将`data/Makefile`文件中 `wget http://downloads.sourceforge.net/libpinyin/models/model19.text.tar.gz` 那行删掉.

### Ubuntu

```
$ sudo apt update
$ sudo apt install gnome-common build-essential autogen libglib2.0-*  libdb-dev
$ export LDFLAGS=`pkg-config glib-2.0 --libs`
$ ./autogen.sh --prefix=/usr
$ make -j32
$ sudo make install
```
> 在20.04.1上测试通过.
