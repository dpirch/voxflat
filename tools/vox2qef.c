#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <voxflat.h>

static const char *progname;

static int vox2qef(FILE *infile, FILE *outfile) {
    VxfError error;
    VxfFile *vf = vxf_open_stream(infile, &error);
    if (!vf) {
        fprintf(stderr, "%s: %s\n", progname, vxf_error_string(error));
        return EXIT_FAILURE;
    }

    fprintf(outfile, "Qubicle Exchange Format\nVersion 0.2\nwww.minddesk.com\n");

    // write scene x/y/z size
    int32_t bounds_min[3], bounds_max[3];
    vxf_calculate_bounds(vf, bounds_min, bounds_max);
    int32_t size_x = bounds_max[0] - bounds_min[0] + 1,
            size_y = bounds_max[1] - bounds_min[1] + 1,
            size_z = bounds_max[2] - bounds_min[2] + 1;
    fprintf(outfile, "%"PRIi32" %"PRIi32" %"PRIi32"\n", size_x, size_y, size_z);

    // write palette (omit index 0)
    uint8_t palette[256][4];
    vxf_get_palette(vf, palette);
    fprintf(outfile, "255\n");
    for (int i = 1; i < 256; i++) {
        double color_r = palette[i][0] / 255.0,
               color_g = palette[i][1] / 255.0,
               color_b = palette[i][2] / 255.0;
        fprintf(outfile, "%.6f %.6f %.6f\n", color_r, color_g, color_b);
    }

    // write voxels
    #define MAX_COUNT 256
    int32_t xyz_buf[MAX_COUNT][3];
    uint8_t color_buf[MAX_COUNT];
    size_t count_read;
    while ((count_read = vxf_read_xyz_coloridx(vf, MAX_COUNT, xyz_buf, color_buf, &error)) > 0) {
        for (size_t i = 0; i < count_read; i++) {
            int32_t pos_x = xyz_buf[i][0] - bounds_min[0],
                    pos_y = xyz_buf[i][1] - bounds_min[1],
                    pos_z = xyz_buf[i][2] - bounds_min[2];
            assert(pos_x >= 0 && pos_y >= 0 && pos_z >= 0);
            if (color_buf[i] == 0) continue;
            fprintf(outfile, "%"PRIi32" %"PRIi32" %"PRIi32" %d 126\n", pos_x, pos_y, pos_z, color_buf[i] - 1);
        }
    }

    vxf_close(vf);

    if (error != VXF_SUCCESS) {
        fprintf(stderr, "%s: %s\n", progname, vxf_error_string(error));
        return EXIT_FAILURE;
    }

    if (ferror(outfile)) {
        fprintf(stderr, "%s: Error while writing to output stream\n", progname);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    progname = *argv && **argv ? *argv : "vox2qef";
    FILE *infile = NULL, *outfile = NULL;
    int retval = EXIT_FAILURE;

    if (argc < 2 || argc > 3) {
        fprintf(stderr,
            "Usage: %s <input.vox> [<output.qef>]\n"
            "  Converts a MagicaVoxel vox file to Qubicle Exchange Format.\n"
            "  If no output filename is specified, writes to stdout.\n",
            progname
        );
        goto end;
    }

    infile = fopen(argv[1], "rb");
    if (!infile) {
        fprintf(stderr, "%s: %s: %s\n", progname, argv[1], errno ? strerror(errno): "Cannot open input file");
        goto end;
    }

    if (argc < 3) {
        outfile = stdout;
    } else if (outfile = fopen(argv[2], "w"), !outfile) {
        fprintf(stderr, "%s: %s: %s\n", progname, argv[2], errno ? strerror(errno): "Cannot open output file");
        goto end;
    }

    retval = vox2qef(infile, outfile);

end:
    if(outfile && outfile != stdout) fclose(outfile);
    if (infile) fclose(infile);
    return retval;
}
