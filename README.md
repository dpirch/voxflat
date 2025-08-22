# voxflat
**voxflat** is a C library for reading [MagicaVoxel](https://ephtracy.github.io/) ".vox" files,
returning the contents as a point cloud of XYZ voxel coordinates and corresponding colors.

The library is written in C11; the header is also compatible with C99 and C++.

Example program printing voxel coordinates and RGBA colors:
```c
#include <voxflat.h>

int main() {
    VxfFile *vf = vxf_open_file("example.vox", NULL);
    int32_t xyz[256][3];
    uint8_t rgba[256][4];
    size_t count;
    while ((count = vxf_read_xyz_rgba(vf, 256, xyz, rgba, NULL)) > 0) {
        for (size_t i = 0; i < count; i++) {
            printf("Voxel at (%d, %d, %d) with color #%02x%02x%02x\n",
                   xyz[i][0], xyz[i][1], xyz[i][2], rgba[i][0], rgba[i][1], rgba[i][2]);
        }
    }
    vxf_close(vf);
}
```

This project also includes two command-line programs, which also serve as examples for using the API
with error handling:
- **vox2qef** for converting vox files into the Qubicle Exchange Format (.qef),
- **vox2txt** for converting vox files into text files that are also supported by [Goxel](https://goxel.xyz/).

These programs expect the input and output filenames as arguments, e.g. `vox2txt input.vox output.txt`.
If the second argument is omitted, the output is written to stdout.

## Features and Limitations
- While vox files can contain scenes with multiple models arranged with geometric transformations, the voxflat
  API abstracts this away and returns all voxels of the scene in a single coordinate system.
- Voxel colors can be returned either as RGBA colors or as palette indices. Material properties
  are not supported. 
- The visibility of model instances and layers is respected; i.e. only what is visible is returned.
- Animations are not supported; the returned scene is that of the first animation frame.
- Data is read directly from the source stream, so reading even large scenes does not require a lot of RAM.
  However, this means that source files must be seekable, since the scene structure is usually stored
  at the end of a vox file. If you need to read vox files from a non-seekable stream, read them into a
  buffer and use `vxf_open_memory()`. 

## Installation
voxflat can be built and installed using [Meson](https://mesonbuild.com/):
```sh
meson setup builddir --buildtype=release
ninja -C builddir install
```
Additional options that can be specified with `meson setup`:
- `--prefix=...` to install to a location other than the default (`/usr/local` on unix)
- `--default_library=static` to create a static instead of the default shared library
- `-D docs=enabled` to install API documentation (requires doxygen)
- `-D tools=enabled` to install the `vox2qef` and `vox2txt` tools.

After installation, you can link the library to your program with `-lvoxflat` (and `-I` and `-L` options
if you installed it to a custom path).

A pkg-config file is also installed, so you can use `pkg-config --cflags --libs voxflat` to get the
required compiler flags, or find it as `dependency('voxflat')` in another meson project or via cmake's PkgConfig
module.

## Using as a meson subproject
To use the library as a subproject in another meson project without installing it, put
```meson
voxflat_proj = subproject('voxflat')
voxflat_dep = voxflat_proj.get_variable('voxflat_dep')
executable('myprogram', 'myprogram.c',  dependencies : [voxflat_dep])
```
into your project's `meson.build`, and either add the contents of the voxflat repository into a
`subprojects/voxflat` directory in your project, or create a `subprojects/voxflat.wrap` file containing
```
[wrap-git]
url = https://github.com/dpirch/voxflat.git
revision = head
depth = 1
```
to fetch the current version from GitHub.
