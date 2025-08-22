#include "common.h"

int main(int argc, char* argv[]) {
    ASSERT_EQ(2, argc);
    FILE *file = fopen(argv[1], "rb");
    ASSERT(file);

    char buffer[100];
    size_t read = fread(buffer, 1, sizeof(buffer), file);
    ASSERT(read > 0 && read < sizeof(buffer));
    fclose(file);

    VxfError error;
    VxfFile *vf = vxf_open_memory(read, buffer, &error);
    ASSERT_EQ(VXF_SUCCESS, error);

    int32_t xyz[5][3];
    uint8_t coloridx[5];

    read = vxf_read_xyz_coloridx(vf, 5, xyz, coloridx, &error);
    ASSERT_EQ(3, read);
    ASSERT_EQ(VXF_SUCCESS, error);

    read = vxf_read_xyz_coloridx(vf, 5, xyz, coloridx, &error);
    ASSERT_EQ(0, read);
    ASSERT_EQ(VXF_SUCCESS, error);

    vxf_close(vf);
}
