The fork
========

Major contributions:
- **Syntax highlighting**: syntax and color scheme definitions using regular expressions, defined in YAML format similar to modern editors.
- **Search (filter) bar redesign**: reduced to single line of control, history dropdown pinning.
- **Multi-window support**: multiple windows in same instance, dragging file tabs between windows.
- **Tab bar improvements**: renaming, coloring, pinning of tabs.
- **Improved keyboard navigation**.

For details look [here](README.fork.md).

## Building

The build was migrated to CMake and currently drops Win32 support.

### Install prerequisites

On Ubuntu:
```sudo apt-get cmake qt5-default libboost-program-options-dev libqt5svg5-dev```

On MacOS:
```brew install cmake llvm boost qt5```

### Compile

For convenience, a simple Makefile is added to automate common tasks (e.g. creating build directories, running *cmake*).
For building release version:

```shell
make submodule_update # fetch Git submodules
make configure_release
make
make install
make cpack # MacOS-only: create DMG image
```

glogg - the fast, smart log explorer
=====================================

glogg is a multi-platform GUI application that helps browse and search
through long and complex log files.  It is designed with programmers and
system administrators in mind and can be seen as a graphical, interactive
combination of grep and less.

## Main features

* Runs on Unix-like systems, Windows and Mac thanks to Qt
* Provides a second window showing the result of the current search
* Reads UTF-8 and ISO-8859-1 files
* Supports grep/egrep like regular expressions
* Colorizes the log and search results
* Displays a context view of where in the log the lines of interest are
* Is fast and reads the file directly from disk, without loading it into memory
* Is open source, released under the GPL

## Download

Installers, binaries and source tarballs are available at https://glogg.bonnefon.org/download.html.

## Requirements

* GCC version 4.8.0 or later
* Qt libraries (version 5.2.0 or later)
* Boost "program-options" development libraries
* Markdown HTML processor (optional, to generate HTML documentation)

glogg version 0.9.X still support older versions of gcc and Qt if you need to
build on an older platform.

## Building

The build system uses qmake. Building and installation is done this way:

```
tar xzf glogg-X.X.X.tar.gz
cd glogg-X.X.X
qmake
make
make install INSTALL_ROOT=/usr/local (as root if needed)
```

`qmake BOOST_PATH=/path/to/boost/` will statically compile the required parts of
the Boost libraries whose source are found at the specified path.
The path should be the directory where the tarball from www.boost.org is
extracted.
(use this method on Windows or if Boost is not available on the system)

The documentation is built and installed automatically if 'markdown'
is found.

## Tests

The tests are built using CMake, and require Qt5 and the Google Mocks source.

```
cd tests
mkdir build
cd build
export QT_DIR=/path/to/qt/if/non/standard
export GMOCK_HOME=/path/to/gmock
cmake ..
make
./glogg_tests
```

## Contact

Please visit glogg's website: http://glogg.bonnefon.org/

The development mailing list is hosted at http://groups.google.co.uk/group/glogg-devel
