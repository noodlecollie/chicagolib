ChicagoLib
==========

A library of various applications which are designed to interface with and run on systems as far back as MS-DOS and Windows 95. These applications are mainly designed to help me work with old Windows systems, particularly with hardware where modern conveniences (USB, CD drives, etc.) are not available.

## Compiling

The following examples apply to Linux. Modify the steps as appropriate for your target platform.

Firstly, set the `WATCOM` environment variable to point to a release of the [Open Watcom](https://github.com/open-watcom/open-watcom-v2/releases) toolchain.

```bash
export WATCOM=/path/to/openwatcom
```

Next, compile the `bootstrap` utility. This is used to generate platform-specific scripts for building the applications.

```bash
# From the repo root:
mkdir build
cd build
bash ../bootstrap/build_linux64.sh
```

Then, use `bootstrap` to generate a build script for your desired application, and run the script to build.

```bash
# From within `build`:
mkdir hlwrld
cd hlwrld
../bootstrap ../../src/hlwrld/hlwrld.bst
bash hlwrld.sh
```
