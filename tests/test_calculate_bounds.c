#include "common.h"

int main(int argc, char* argv[]) {
    ASSERT_EQ(8, argc);
    const char *filename = argv[1];
    int32_t expected_min[3] = {atoi(argv[2]), atoi(argv[3]), atoi(argv[4])};
    int32_t expected_max[3] = {atoi(argv[5]), atoi(argv[6]), atoi(argv[7])};

    VxfError error;
    VxfFile *vf = vxf_open_file(filename, &error);
    ASSERT_EQ(VXF_SUCCESS, error);

    int32_t min[3], max[3];

    vxf_calculate_bounds(vf, min, max);
    ASSERT_EQ(expected_min[0], min[0]);
    ASSERT_EQ(expected_min[1], min[1]);
    ASSERT_EQ(expected_min[2], min[2]);
    ASSERT_EQ(expected_max[0], max[0]);
    ASSERT_EQ(expected_max[1], max[1]);
    ASSERT_EQ(expected_max[2], max[2]);

    vxf_close(vf);
}
