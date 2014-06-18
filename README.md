lib-autopin-plus
================

Project directories and files
------------

    build		Build directory (initially empty)
    doc			Documentation of the code
    src			Source code of autopin+_linuxC

    CMakeLists.txt	CMake configuration file
    Doxyfile		Configuration file for Doxygen
    CHANGELOG		Changelog for autopin+_linuxC
    README		This README

Building autopin+_linuxC
------------

    The autopin+ communication library autopin+_linuxC has been developed for Linux and uses
    CMake for compiling. In the following, it will be assumed that ccmake is used. In order
    to compile the library change to the build directory and execute

      ccmake ..

    Then type "c" to configure the project. If the configuration process has finished without
    errors the STATIC option can be set to "ON" to compile a static library. By default, STATIC
    is set to "OFF" and a shared library is built. CMAKE_BUILD_TYPE should be set to "Release".
    If necessary, type "c" again and then type "g" to create the build files and exit the
    application. Now, the library can be built by executing

      make

    in the build directory.

Creating the documentation
------------

    After configuring the build system with CMake the documentation can be refreshed by
    executing

      make doc

    in the build directory. This only works when Doxygen and graphviz are installed. The
    documentation can be deleted by executing

      make cleandoc

Cleaning the project
------------

    Intermedia build results can be deleted with the command

      make clean

    in the build directory.
