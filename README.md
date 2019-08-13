# Analysis

Analysis software for the WAGASCI experiment.

## CMAKE : Compile and install

Recently the WAGASCI Analysis software has switched from Make to CMake to
support multiple OS and platforms, in particular MacOS and Linux.  To compile
and install with CMake create a build directory somewhere and issue the
following commands.

```
cd Analysis
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../
make -j4
make install

```

In the example above the build directory is called `build` and is created just
inside the `Analysis` folder. If you place the build directory somewhere else,
substitute the double dots `..` with the correct path to the `Analysis`
directory. If you want, ou can modify the `CMAKE_INSTALL_PREFIX` too.

To uninstall, move to the build directory and issue:

```
xargs rm < install_manifest.txt
```

## Documentation

To compile the documentation move to the `Analysis` directory and `make doc`.
The documentation is build with
[Sphinx](https://www.sphinx-doc.org/en/master/). It can be read with any Web
browser by opening the file `Analysis/doc/build/index.html`.

## Dependencies : Linux

### ROOT
The `Analysis` code depends heavily on CERN
[ROOT](https://root.cern.ch/). Please install it and be sure that the `ROOTSYS`
variable (and all the ROOT environment) are correctly set before compiling the
`Analysis` code.

### BOOST
You also need to install boost libraries (at least `system` and
`filesystem`). Boost is always present in the repositories of every major Linux
distribution. For example in Ubuntu you just need to

```
sudo apt-get install libboost-all-dev
```

And in Centos just

```
yum install boost-devel
```

In MacOS download [MacPorts](https://www.macports.org/), and run the following
command

```
sudo apt install cmake
```

RHEL/CentOS

```
sudo yum install cmake
```

### nlohmann-json

Debian/Ubuntu
```
sudo apt install nlohmann-json-dev
```

### Sphinx (optional)

Debian/Ubuntu:

```
sudo apt-get install python3-sphinx
```

RHEL/CentOS:

```
sudo yum install python-sphinx
```

## Dependencies : MacOS

### ROOT

Download the latest ROOT binary (.dmg) for MacOS from
[here](https://root.cern.ch/downloading-root) and install it. No other
configuration is necessary.

### XCode command line tools
Open a terminal and issue this command

```
sudo xcode-select --install
```

### MacPorts

You can download the automatic installer from here
[MacPorts](https://www.macports.org/),

### Boost

After installing MacPorts, run the following command

```
sudo port install boost
```

### Fix "<bits/stdc++.h> not found" error

```
sudo port install wget
cd /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1
mkdir bits
cd bits
wget https://gist.githubusercontent.com/reza-ryte-club/97c39f35dab0c45a5d924dd9e50c445f/raw/47ecad34033f986b0972cdbf4636e22f838a1313/stdc++.h
```

The instructions above were taken from
[here](https://qiita.com/acchan_ar/items/6a4c4c070dd76a236fdc) (Only Japanese).

### CMake

Download the auto-installer (.dmg) from [here](https://cmake.org/download/).

### nlohmann-json

```
sudo port install nlohmann-json
```

### Sphinx (optional)

Make sure that MacPorts is installed and then:

```
sudo port install py36-sphinx
sudo port select --set python python36
sudo port select --set sphinx py36-sphinx
```