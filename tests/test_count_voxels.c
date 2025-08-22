#include "common.h"

int main(int argc, char* argv[]) {
    ASSERT_EQ(3, argc);
    const char *filename = argv[1];
    unsigned long expected_count = strtoul(argv[2], NULL, 10);

    VxfError error;
    VxfFile *vf = vxf_open_file(filename, &error);
    ASSERT_EQ(VXF_SUCCESS, error);

    uint64_t voxel_count = vxf_count_voxels(vf);
    ASSERT_EQ(expected_count, voxel_count);

    vxf_close(vf);
}
