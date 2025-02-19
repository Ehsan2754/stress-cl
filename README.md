# stress-cl

![Build and Release](https://github.com/ehsan2754/stress-cl/actions/workflows/release.yml/badge.svg)
`stress-cl` is tool that allows runing a stress test on a specified OpenCL device, and provides help information.

## Features

- List OpenCL devices with the `-l` option.
- Run a stress test on a specified device with the `-d` option and for a specified duration with the `-t` option.
- Print help information with the `-h` option.
- Print version information with the `--version` option.

## Usage
Options
- `-l` List OpenCL devices
- `-d <index>` Run stress test on device with given index
- `-t <time>` Duration of the stress test in seconds
- `-h` Print this help message
- `-v` Print version information
## Building
To build the project, you need CMake and a C++ compiler. Follow these steps:
``` mkdir build; cd build; cmake ..; make ```
## Packaging
``` make package ```

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Maintainer

Ehsan Shaghaei (contact@byehsan.com)
