# libpinyin

Library to deal with pinyin.

The libpinyin project aims to provide the algorithms core for intelligent sentence-based Chinese pinyin input methods.

## 编译安装

发行版中自带版本较老, 如果需要安装新版可以从源码编译.

### Ubuntu

系统版本: 20.04.1

```
$ sudo apt install gnome-common build-essential autogen
$ sudo apt install libibus-1.0-dev libsqlite3-dev sqlite3 libdb-dev libpinyin13-dev

$ git clone -depth=1 https://github.com/libpinyin/libpinyin.git
$ cd ./libpinyin
$ export LDFLAGS=`pkg-config glib-2.0 --libs`
$ ./autogen.sh --prefix=/usr
```

libpinyin编译时需要下载[model19.text.tar.gz](http://downloads.sourceforge.net/libpinyin/models/model19.text.tar.gz), 但是sourceforge国内访问速度极慢! 建议事先科学上网将其下载后将其放到data目录中, 然后将`data/Makefile`文件中 `wget http://downloads.sourceforge.net/libpinyin/models/model19.text.tar.gz` 那行删掉.

```
$ make -j32
$ sudo make install
```
