# V4L2-Stream

Stream V4L2 camera to Web Browser

## Prerequisites

* gstreamer1.0
* libwebsocketpp
* libboost-system
* pkg-config
* build-essential
* python3 (optional, for a simple web server hosting static files)

Install with

```
$ sudo apt install libgstreamer1.0-dev libwebsocketpp-dev pkg-config build-essential libboost-system-dev python3
```

## Download & Build

```
$ git clone https://github.com/mbyzhang/v4l2-stream.git
$ cd v4l2-stream
$ make
```

## Usage

Start stream server with

```
$ ./v4l2-streamer 
```

and start Web server with

```
$ cd v4l2-stream/public
$ python3 -m http.server
```

and open [http://localhost:8000/](http://localhost:8000/) to see the stream.

## Third-party notices

```
Broadway.js - https://github.com/mbebenita/Broadway

Copyright (c) 2011, Project Authors (see AUTHORS file)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  *  Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  *  Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  *  Neither the names of the Project Authors nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

--

The 3-clause BSD above applies to all code except for code originating
from the Android project (the .cpp files in Avc/). Those files are under
the Android project's Apache 2.0 license.
```
