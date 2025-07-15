# bin2shellcode

Simple tool for converting .bin files into different shellcode formats.
The conversion logic is available as a static library so it can be reused for other projects.

```
bin2shellcode 1.0.0
=======================

Usage:
  bin2shellcode --input <path> --format <n> [--varname <name>]
  bin2shellcode <path> <n> [<name>]

Options:
  -i, --input <path>      Path to the .bin to convert (required)
  -f, --format <n>        Output format (1-11) (required)
  -n, --varname <name>    Name of the output variable (default: shellcode)
  -h, --help              Show this help and exit
  -v, --version           Show version and exit

Formats:
  #   Description
  -------------------------------------------------------
  1   Raw hex bytes                   (fc 48 81 ...)
  2   Base64 string                   (SGVsbG8uLi4=)
  3   C                               (unsigned char shellcode[] = "\x90\x90...")
  4   Ruby                            (unsigned char buf[] = "\x90\x90...")
  5   Python                          (shellcode = b"\x90\x90...")
  6   PowerShell                      ([Byte[]] $shellcode = 0x90,0x90,...)
  7   C#                              (byte[] shellcode = { 0x90, 0x90, ... })
  8   Hex string                      (9090...)
  9   UUID list                       (fc4881e4-f0ff-ffff-...)
  10  Go                              ([]byte{0x90, 0x90, ...})
  11  Rust                            (let shellcode: &[u8] = &[0x90, 0x90, ...];)

Examples:
  Convert "payload.bin" to a C byte array:
      bin2shellcode -i payload.bin -f 1
```

## Linux dependencies

`cmake` and a C++ compiler (GCC or Clang) are required. On Debian based
systems they can be installed with:

```bash
sudo apt install build-essential cmake
```

On Arch Linux install the required packages with:

```bash
sudo pacman -S base-devel cmake
```

## Building on Linux

```bash
mkdir build -p && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Create a `.deb` package

```bash
cmake --build . --target package
```

## Installing on Linux (Admin required)

From the *build* directory run:

```bash
sudo cmake --install .
```

This copies the binary and library to `/usr/local` by default.
Remember to run this command as administrator in order to install the programs.

## Windows dependencies

Install the required tools with [Scoop](https://scoop.sh) (Recomended): 

```powershell
scoop install llvm cmake ninja
```

## Building on Windows

```powershell
mkdir build -Force ; cd build
cmake -S .. -G Ninja -DCMAKE_CXX_COMPILER="clang++.exe" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="C:/Program Files/bin2shellcode"
cmake --build .
```

## Installing on Windows (Admin required)

Run the installation from an elevated PowerShell prompt:

```powershell
cmake --install . --prefix "C:/Program Files/bin2shellcode"
```

The executable is copied to `C:\Program Files\bin2shellcode\bin`
together with `libbin2shellcode.lib`. To permanently add the folder to the `PATH`
you can run:

```powershell
setx PATH "$Env:PATH;C:\Program Files\bin2shellcode\bin"
```

## Running the tests

To compile and run the tests from *build* use `CTest`.
Make sure to configure the project in **Debug** mode so the `assert`
statements in the test program are executed:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
cmake --build . --target test
ctest --output-on-failure
```

## Uninstall (Admin required)

```bash
cmake --build . --target uninstall
```

## Cleanup

```bash
cmake --build . --target clean-build
```
