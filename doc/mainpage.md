See [voxflat.h](@ref voxflat.h) for the API documentation.

### API Overview
To use the voxflat library to process a MagicaVoxel .vox file, follow these steps:

1. **Open the file**: Use @ref vxf_open_file, @ref vxf_open_stream, or @ref vxf_open_memory
   to create a @ref VxfFile instance from a file, stdio stream, or memory buffer.
2. **Query scene information** (optional):
   - Use @ref vxf_calculate_bounds to get the bounding box of the voxel coordinates.
   - Use @ref vxf_count_voxels to get the total number of voxels.
   - Use @ref vxf_get_palette to retrieve the color palette.
3. **Read voxel data**: Call @ref vxf_read_xyz_rgba or @ref vxf_read_xyz_coloridx repeatedly to iterate
   over the voxels, retrieving their positions and colors or color palette indices.
4. **Close the file**: Call @ref vxf_close to free the VxfFile instance and associated resources.

### Example Program
The following example opens a .vox file, retrieves its color palette, and prints the position and RGB color of each voxel.
It assumes the file is valid and focuses on simplicity for illustrative purposes.

```c
#include <voxflat.h>

int main() {
    // Open the vox file
    VxfFile *vf = vxf_open_file("example.vox", NULL);

    // Read and print voxel data
    int32_t xyz[256][3];
    uint8_t rgba[256][4];
    size_t count;
    while ((count = vxf_read_xyz_rgba(vf, 256, xyz, rgba, NULL)) > 0) {
        for (size_t i = 0; i < count; i++) {
            printf("Voxel at (%d, %d, %d) with color #%02x%02x%02x\n",
                   xyz[i][0], xyz[i][1], xyz[i][2],
                   rgba[i][0], rgba[i][1], rgba[i][2]);
        }
    }

    // Clean up
    vxf_close(vf);
}
```
