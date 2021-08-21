# Private web sever for Bibi / Bibi専用プライベートWebサーバー

## Overview

This is a web server that only works with [Bibi](https://github.com/satorumurmur/bibi).

Bibi is the wonderful EPUB Reader!

### Supported Platforms

This server has been tested on the following platforms:

- Windows 10
- Ubuntu 20.04

## How to build

### QtHttpServer(Required)

```
$ git clone git@github.com:qt-labs/qthttpserver.git
$ cd qthttpserver
$ git submodule update --init
$ cd ..
$ mkdir build-qthttpserver
$ cd build-qthttpserver
$ qmake ../qthttpserver
$ make
$ make install
```

### Node.js(Required)

See [Node.js web site](https://nodejs.org/).

LTS version is required(14.17.5).
(I couldn't build the latest version)

### Private web sever for Bibi

```
$ git clone git@github.com:ioriayane/privatewebserver4bibi.git
$ cd privatewebserver4bibi
$ git submodule update --init

$ cd bibi
$ npm install
$ npm run build
```

Please build using Qt Creator.

## How to use

### Start server

```
$ ./privatewebserver4bibi --book-shelf <FOLDER_PATH> [--port <NO>]
```

Open the following URL in your browser.

`http://localhost:52224/bibi/index.html?book=example.epub`

### Stop server

```
$ ./privatewebserver4bibi --stop
```

### from GUI application

See [this project](./example).

## License

- Private web sever for Bibi (C) Takayuki Orito

It uses the following OSS:

- [Bibi](https://github.com/satorumurmur/bibi) (C) Satoru Matsushima (MIT License)
- [Qt](https://www.qt.io/) (C) The Qt Company Ltd. (GPL3, LGPL3)
