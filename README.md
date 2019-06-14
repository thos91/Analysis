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
cmake .. cmake .. -DCMAKE_INSTALL_PREFIX=../
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

## Dependencies

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
sudo port install boost
```

## (Obsolete) MAKE : Compile and install

If you are on Linux you can use the old Make to compile the source code.  Just
move to the `Analysis` directory and issue `make -j4`.  The binaries and
libraries will be all located in the `Analysis/bin` directory. There is no
install target.
