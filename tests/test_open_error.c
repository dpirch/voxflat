#include "common.h"

int main(int argc, char* argv[]) {
    ASSERT_EQ(3, argc);
    const char *filename = argv[1];
    unsigned expected_error = atoi(argv[2]);

    VxfError error;
    VxfFile *vf = vxf_open_file(filename, &error);

    ASSERT_EQ(expected_error, error);
    ASSERT(vf == NULL);
}
