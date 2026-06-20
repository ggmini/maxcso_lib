Description
===========

A fork of the MaxCSO application by Unkown Brackets, as a library to be integrated into other applications.
Includes a little application to test if the library actually works.

> [!WARNING]
> This project is still a work in progress and is not ready to be used as a library yet. Certain functionality has yet to be exposed through the library. You may be able to make it work, if you dig deep enough, but some of these things will likely be changed in the future.

> [!IMPORTANT]
> While I'm developing for Windows and Linux (through MSVC and Make respectively), there is a chance that some updates will only be tested/available on Linux a bit later as I mainly develop on Windows. I'll probably try to fix this with workflows, but that doesn't account for updates to the Makefile, etc.


Credits and licensing
===========

The larger portion of code here is from others' wonderful work in decompression and I/O
libraries.  Licensing is as follows:

 * [maxcso][] is licensed under ISC.
 * [7-zip][] and [p7zip][] are licensed under LGPL.
 * [Zopfli][] is licensed under Apache 2.0.
 * [libuv][] and [libdeflate][] are licensed under MIT.
 * [zlib][] is licensed under zlib.
 * [lz4][] is licensed under BSD.


Other tools
===========

 * [CisoPlus][] by kapoue3
 * [CisoMC][] by LMAN
 * [ciso][] by BOOSTER
 * [ciso-python][] by Virtuous Flame

[maxcso]: https://github.com/unknownbrackets/maxcso
[zlib]: https://github.com/madler/zlib
[7-zip]: http://7-zip.org/
[p7zip]: http://p7zip.sourceforge.net/
[Zopfli]: https://github.com/google/zopfli
[libuv]: https://github.com/joyent/libuv
[libdeflate]: https://github.com/ebiggers/libdeflate
[CisoPlus]: https://web.archive.org/web/20161223115412/http://cisoplus.pspgen.com/
[CisoMC]: http://wololo.net/talk/viewtopic.php?f=20&t=32659
[ciso]: http://sourceforge.net/projects/ciso/
[ciso-python]: https://github.com/MrColdbird/procfw/blob/master/contrib/ciso.py
[lz4]: https://github.com/lz4/lz4
[CSO v2]: README_CSO.md
[ZSO]: README_ZSO.md
