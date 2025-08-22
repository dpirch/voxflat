#include "common.h"

int main(int argc, char* argv[]) {
    ASSERT_EQ(2, argc);
    VxfError error;
    FILE *file = fopen(argv[1], "rb");
    ASSERT(file);

    VxfFile *vf = vxf_open_stream(file, &error);
    ASSERT_EQ(VXF_SUCCESS, error);

    int32_t xyz[2][3];
    uint8_t coloridx[2];

    size_t result = vxf_read_xyz_coloridx(vf, 2, xyz, coloridx, &error);
    ASSERT_EQ(2, result);
    ASSERT_EQ(VXF_SUCCESS, error);

    // change file position to cause an error
    fseek(file, 0, SEEK_END);

    result = vxf_read_xyz_coloridx(vf, 2, xyz, coloridx, &error);
    ASSERT_EQ(0, result);
    ASSERT_EQ(VXF_ERROR_UNEXPECTED_EOF, error);

    // trying again just returns same error
    result = vxf_read_xyz_coloridx(vf, 2, xyz, coloridx, &error);
    ASSERT_EQ(0, result);
    ASSERT_EQ(VXF_ERROR_UNEXPECTED_EOF, error);

    vxf_close(vf);
    fclose(file);
}
