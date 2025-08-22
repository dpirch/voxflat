#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <voxflat.h>

static const char *progname;

static int vox2txt(FILE *infile, FILE *outfile) {
    VxfError error;
    VxfFile *vf = vxf_open_stream(infile, &error);
    if (!vf) {
        fprintf(stderr, "%s: %s\n", progname, vxf_error_string(error));
        return EXIT_FAILURE;
    }

    fprintf(outfile, "# X Y Z RRGGBB\n");

    #define MAX_COUNT 256
    int32_t xyz[MAX_COUNT][3];
    uint8_t rgba[MAX_COUNT][4];
    size_t count_read;
    while ((count_read = vxf_read_xyz_rgba(vf, MAX_COUNT, xyz, rgba, &error)) > 0) {
        for (size_t i = 0; i < count_read; i++) {
            fprintf(outfile, "%"PRIi32" %"PRIi32" %"PRIi32" %02x%02x%02x\n",
                xyz[i][0], xyz[i][1], xyz[i][2], rgba[i][0], rgba[i][1], rgba[i][2]);
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
    progname = *argv && **argv ? *argv : "vox2txt";
    FILE *infile = NULL, *outfile = NULL;
    int retval = EXIT_FAILURE;

    if (argc < 2 || argc > 3) {
        fprintf(stderr,
            "Usage: %s <input.vox> [<output.txt>]\n"
            "  Converts a MagicaVoxel vox file to text (in the format also supported by Goxel).\n"
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

    retval = vox2txt(infile, outfile);

end:
    if(outfile && outfile != stdout) fclose(outfile);
    if (infile) fclose(infile);
    return retval;
}
