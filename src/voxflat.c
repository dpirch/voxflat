#include <voxflat.h>
#include <string.h>
#include <setjmp.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <limits.h>
#include <stdnoreturn.h>
#include <stdbool.h>

#ifndef unreachable
#define unreachable() do { assert(0 && "Unreachable code"); for (;;); } while (0)
#endif

#define GET_BYTES_MAX 1024 // enough for palette data

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define CLAMP(x, min, max) MIN(MAX((x), (min)), (max))

#define FOURCC(a,b,c,d) ((uint32_t) (((d) << 24) | ((c) << 16) | ((b) << 8) | (a)))

#define FOURCC_VOX  FOURCC('V','O','X',' ')
#define FOURCC_MAIN FOURCC('M','A','I','N')
#define FOURCC_SIZE FOURCC('S','I','Z','E')
#define FOURCC_XYZI FOURCC('X','Y','Z','I')
#define FOURCC_RGBA FOURCC('R','G','B','A')
#define FOURCC_nGRP FOURCC('n','G','R','P')
#define FOURCC_nTRN FOURCC('n','T','R','N')
#define FOURCC_nSHP FOURCC('n','S','H','P')
#define FOURCC_LAYR FOURCC('L','A','Y','R')

static const uint8_t default_palette[256][4] = {
    {0x00,0x00,0x00,0x00}, {0xff,0xff,0xff,0xff}, {0xff,0xff,0xcc,0xff}, {0xff,0xff,0x99,0xff}, {0xff,0xff,0x66,0xff}, {0xff,0xff,0x33,0xff}, {0xff,0xff,0x00,0xff}, {0xff,0xcc,0xff,0xff},
    {0xff,0xcc,0xcc,0xff}, {0xff,0xcc,0x99,0xff}, {0xff,0xcc,0x66,0xff}, {0xff,0xcc,0x33,0xff}, {0xff,0xcc,0x00,0xff}, {0xff,0x99,0xff,0xff}, {0xff,0x99,0xcc,0xff}, {0xff,0x99,0x99,0xff},
    {0xff,0x99,0x66,0xff}, {0xff,0x99,0x33,0xff}, {0xff,0x99,0x00,0xff}, {0xff,0x66,0xff,0xff}, {0xff,0x66,0xcc,0xff}, {0xff,0x66,0x99,0xff}, {0xff,0x66,0x66,0xff}, {0xff,0x66,0x33,0xff},
    {0xff,0x66,0x00,0xff}, {0xff,0x33,0xff,0xff}, {0xff,0x33,0xcc,0xff}, {0xff,0x33,0x99,0xff}, {0xff,0x33,0x66,0xff}, {0xff,0x33,0x33,0xff}, {0xff,0x33,0x00,0xff}, {0xff,0x00,0xff,0xff},
    {0xff,0x00,0xcc,0xff}, {0xff,0x00,0x99,0xff}, {0xff,0x00,0x66,0xff}, {0xff,0x00,0x33,0xff}, {0xff,0x00,0x00,0xff}, {0xcc,0xff,0xff,0xff}, {0xcc,0xff,0xcc,0xff}, {0xcc,0xff,0x99,0xff},
    {0xcc,0xff,0x66,0xff}, {0xcc,0xff,0x33,0xff}, {0xcc,0xff,0x00,0xff}, {0xcc,0xcc,0xff,0xff}, {0xcc,0xcc,0xcc,0xff}, {0xcc,0xcc,0x99,0xff}, {0xcc,0xcc,0x66,0xff}, {0xcc,0xcc,0x33,0xff},
    {0xcc,0xcc,0x00,0xff}, {0xcc,0x99,0xff,0xff}, {0xcc,0x99,0xcc,0xff}, {0xcc,0x99,0x99,0xff}, {0xcc,0x99,0x66,0xff}, {0xcc,0x99,0x33,0xff}, {0xcc,0x99,0x00,0xff}, {0xcc,0x66,0xff,0xff},
    {0xcc,0x66,0xcc,0xff}, {0xcc,0x66,0x99,0xff}, {0xcc,0x66,0x66,0xff}, {0xcc,0x66,0x33,0xff}, {0xcc,0x66,0x00,0xff}, {0xcc,0x33,0xff,0xff}, {0xcc,0x33,0xcc,0xff}, {0xcc,0x33,0x99,0xff},
    {0xcc,0x33,0x66,0xff}, {0xcc,0x33,0x33,0xff}, {0xcc,0x33,0x00,0xff}, {0xcc,0x00,0xff,0xff}, {0xcc,0x00,0xcc,0xff}, {0xcc,0x00,0x99,0xff}, {0xcc,0x00,0x66,0xff}, {0xcc,0x00,0x33,0xff},
    {0xcc,0x00,0x00,0xff}, {0x99,0xff,0xff,0xff}, {0x99,0xff,0xcc,0xff}, {0x99,0xff,0x99,0xff}, {0x99,0xff,0x66,0xff}, {0x99,0xff,0x33,0xff}, {0x99,0xff,0x00,0xff}, {0x99,0xcc,0xff,0xff},
    {0x99,0xcc,0xcc,0xff}, {0x99,0xcc,0x99,0xff}, {0x99,0xcc,0x66,0xff}, {0x99,0xcc,0x33,0xff}, {0x99,0xcc,0x00,0xff}, {0x99,0x99,0xff,0xff}, {0x99,0x99,0xcc,0xff}, {0x99,0x99,0x99,0xff},
    {0x99,0x99,0x66,0xff}, {0x99,0x99,0x33,0xff}, {0x99,0x99,0x00,0xff}, {0x99,0x66,0xff,0xff}, {0x99,0x66,0xcc,0xff}, {0x99,0x66,0x99,0xff}, {0x99,0x66,0x66,0xff}, {0x99,0x66,0x33,0xff},
    {0x99,0x66,0x00,0xff}, {0x99,0x33,0xff,0xff}, {0x99,0x33,0xcc,0xff}, {0x99,0x33,0x99,0xff}, {0x99,0x33,0x66,0xff}, {0x99,0x33,0x33,0xff}, {0x99,0x33,0x00,0xff}, {0x99,0x00,0xff,0xff},
    {0x99,0x00,0xcc,0xff}, {0x99,0x00,0x99,0xff}, {0x99,0x00,0x66,0xff}, {0x99,0x00,0x33,0xff}, {0x99,0x00,0x00,0xff}, {0x66,0xff,0xff,0xff}, {0x66,0xff,0xcc,0xff}, {0x66,0xff,0x99,0xff},
    {0x66,0xff,0x66,0xff}, {0x66,0xff,0x33,0xff}, {0x66,0xff,0x00,0xff}, {0x66,0xcc,0xff,0xff}, {0x66,0xcc,0xcc,0xff}, {0x66,0xcc,0x99,0xff}, {0x66,0xcc,0x66,0xff}, {0x66,0xcc,0x33,0xff},
    {0x66,0xcc,0x00,0xff}, {0x66,0x99,0xff,0xff}, {0x66,0x99,0xcc,0xff}, {0x66,0x99,0x99,0xff}, {0x66,0x99,0x66,0xff}, {0x66,0x99,0x33,0xff}, {0x66,0x99,0x00,0xff}, {0x66,0x66,0xff,0xff},
    {0x66,0x66,0xcc,0xff}, {0x66,0x66,0x99,0xff}, {0x66,0x66,0x66,0xff}, {0x66,0x66,0x33,0xff}, {0x66,0x66,0x00,0xff}, {0x66,0x33,0xff,0xff}, {0x66,0x33,0xcc,0xff}, {0x66,0x33,0x99,0xff},
    {0x66,0x33,0x66,0xff}, {0x66,0x33,0x33,0xff}, {0x66,0x33,0x00,0xff}, {0x66,0x00,0xff,0xff}, {0x66,0x00,0xcc,0xff}, {0x66,0x00,0x99,0xff}, {0x66,0x00,0x66,0xff}, {0x66,0x00,0x33,0xff},
    {0x66,0x00,0x00,0xff}, {0x33,0xff,0xff,0xff}, {0x33,0xff,0xcc,0xff}, {0x33,0xff,0x99,0xff}, {0x33,0xff,0x66,0xff}, {0x33,0xff,0x33,0xff}, {0x33,0xff,0x00,0xff}, {0x33,0xcc,0xff,0xff},
    {0x33,0xcc,0xcc,0xff}, {0x33,0xcc,0x99,0xff}, {0x33,0xcc,0x66,0xff}, {0x33,0xcc,0x33,0xff}, {0x33,0xcc,0x00,0xff}, {0x33,0x99,0xff,0xff}, {0x33,0x99,0xcc,0xff}, {0x33,0x99,0x99,0xff},
    {0x33,0x99,0x66,0xff}, {0x33,0x99,0x33,0xff}, {0x33,0x99,0x00,0xff}, {0x33,0x66,0xff,0xff}, {0x33,0x66,0xcc,0xff}, {0x33,0x66,0x99,0xff}, {0x33,0x66,0x66,0xff}, {0x33,0x66,0x33,0xff},
    {0x33,0x66,0x00,0xff}, {0x33,0x33,0xff,0xff}, {0x33,0x33,0xcc,0xff}, {0x33,0x33,0x99,0xff}, {0x33,0x33,0x66,0xff}, {0x33,0x33,0x33,0xff}, {0x33,0x33,0x00,0xff}, {0x33,0x00,0xff,0xff},
    {0x33,0x00,0xcc,0xff}, {0x33,0x00,0x99,0xff}, {0x33,0x00,0x66,0xff}, {0x33,0x00,0x33,0xff}, {0x33,0x00,0x00,0xff}, {0x00,0xff,0xff,0xff}, {0x00,0xff,0xcc,0xff}, {0x00,0xff,0x99,0xff},
    {0x00,0xff,0x66,0xff}, {0x00,0xff,0x33,0xff}, {0x00,0xff,0x00,0xff}, {0x00,0xcc,0xff,0xff}, {0x00,0xcc,0xcc,0xff}, {0x00,0xcc,0x99,0xff}, {0x00,0xcc,0x66,0xff}, {0x00,0xcc,0x33,0xff},
    {0x00,0xcc,0x00,0xff}, {0x00,0x99,0xff,0xff}, {0x00,0x99,0xcc,0xff}, {0x00,0x99,0x99,0xff}, {0x00,0x99,0x66,0xff}, {0x00,0x99,0x33,0xff}, {0x00,0x99,0x00,0xff}, {0x00,0x66,0xff,0xff},
    {0x00,0x66,0xcc,0xff}, {0x00,0x66,0x99,0xff}, {0x00,0x66,0x66,0xff}, {0x00,0x66,0x33,0xff}, {0x00,0x66,0x00,0xff}, {0x00,0x33,0xff,0xff}, {0x00,0x33,0xcc,0xff}, {0x00,0x33,0x99,0xff},
    {0x00,0x33,0x66,0xff}, {0x00,0x33,0x33,0xff}, {0x00,0x33,0x00,0xff}, {0x00,0x00,0xff,0xff}, {0x00,0x00,0xcc,0xff}, {0x00,0x00,0x99,0xff}, {0x00,0x00,0x66,0xff}, {0x00,0x00,0x33,0xff},
    {0xee,0x00,0x00,0xff}, {0xdd,0x00,0x00,0xff}, {0xbb,0x00,0x00,0xff}, {0xaa,0x00,0x00,0xff}, {0x88,0x00,0x00,0xff}, {0x77,0x00,0x00,0xff}, {0x55,0x00,0x00,0xff}, {0x44,0x00,0x00,0xff},
    {0x22,0x00,0x00,0xff}, {0x11,0x00,0x00,0xff}, {0x00,0xee,0x00,0xff}, {0x00,0xdd,0x00,0xff}, {0x00,0xbb,0x00,0xff}, {0x00,0xaa,0x00,0xff}, {0x00,0x88,0x00,0xff}, {0x00,0x77,0x00,0xff},
    {0x00,0x55,0x00,0xff}, {0x00,0x44,0x00,0xff}, {0x00,0x22,0x00,0xff}, {0x00,0x11,0x00,0xff}, {0x00,0x00,0xee,0xff}, {0x00,0x00,0xdd,0xff}, {0x00,0x00,0xbb,0xff}, {0x00,0x00,0xaa,0xff},
    {0x00,0x00,0x88,0xff}, {0x00,0x00,0x77,0xff}, {0x00,0x00,0x55,0xff}, {0x00,0x00,0x44,0xff}, {0x00,0x00,0x22,0xff}, {0x00,0x00,0x11,0xff}, {0xee,0xee,0xee,0xff}, {0xdd,0xdd,0xdd,0xff},
    {0xbb,0xbb,0xbb,0xff}, {0xaa,0xaa,0xaa,0xff}, {0x88,0x88,0x88,0xff}, {0x77,0x77,0x77,0xff}, {0x55,0x55,0x55,0xff}, {0x44,0x44,0x44,0xff}, {0x22,0x22,0x22,0xff}, {0x11,0x11,0x11,0xff},
};

static uint32_t load_u32(const char bytes[4]) {
    uint32_t result;
    #if __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_LITTLE__
    memcpy(&result, bytes, sizeof(result));
    #else
    auto b = (const uint8_t*)bytes;
    result = (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
    #endif
    return result;
}

static int32_t load_i32(const char bytes[4]) {
    int32_t result;
    #if __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_LITTLE__
    memcpy(&result, bytes, sizeof(result));
    #else
    uint32_t u = load_u32(bytes);
    memcpy(&result, &u, sizeof(result));
    #endif
    return result;
}

struct retjmp { jmp_buf jump; VxfError error; };

static noreturn void return_error(struct retjmp *retjmp, VxfError error) {
    retjmp->error = error;
    longjmp(retjmp->jump, 1);
}

static void *xcalloc(size_t nmemb, size_t size, struct retjmp *retjmp) {
    assert(size > 0);
    void *result = calloc(nmemb, size);
    if (!result && nmemb > 0) return_error(retjmp, VXF_ERROR_OUT_OF_MEMORY);
    return result;
}

#define Array(type) struct { type *items; size_t len, capacity; }

#define ARRAY_APPEND(array, retjmp) \
    ((array).items = (array).len < (array).capacity ? (array).items : \
        array_increase_capacity((array).items, sizeof *(array).items, &(array).capacity, &(retjmp)), \
    &(array).items[(array).len++])

static void *array_increase_capacity(void *items, size_t itemsize, size_t *capacity, struct retjmp *retjmp) {
    if (*capacity > SIZE_MAX / itemsize / 2) goto fail;
    size_t newcap = *capacity < 16 ? 16 : *capacity * 2;
    items = realloc(items, newcap * itemsize);
    if (items) return *capacity = newcap, items;
fail:
    return_error(retjmp, VXF_ERROR_OUT_OF_MEMORY);
}

struct transform {
    uint8_t rotation_cols[3]; // index of nonzero column (0, 1 or 2) per row in rotation matrix
    int8_t rotation_signs[3]; // sign (-1 or 1) of per row in rotation matrix
    int32_t translation[3];
};

#define TRANSFORM_IDENTITY (struct transform){.rotation_cols = {0, 1, 2}, .rotation_signs = {1, 1, 1}}

static struct transform combine_transforms(const struct transform *a, const struct transform *b) {
    struct transform result;
    for (int i = 0; i < 3; i++) {
        result.rotation_cols[i] = b->rotation_cols[a->rotation_cols[i]];
        result.rotation_signs[i] = a->rotation_signs[i] * b->rotation_signs[a->rotation_cols[i]];
        result.translation[i] = a->translation[i] + b->translation[a->rotation_cols[i]] * a->rotation_signs[i];
    }
    return result;
}

static void apply_transform(const struct transform *transform, const uint8_t modelpos[3], int32_t globalpos[3]) {
    for (int i = 0; i < 3; i++) {
        globalpos[i] = modelpos[transform->rotation_cols[i]] * transform->rotation_signs[i] + transform->translation[i];
    }
}

struct model_size {
    uint32_t size[3];
};

static struct transform get_model_transform(const struct transform *parent, const struct model_size *size) {
    struct transform result = *parent;
    for (int i = 0; i < 3; i++) {
        result.translation[i] -= (int32_t)(size->size[parent->rotation_cols[i]] / 2) * parent->rotation_signs[i];
        if (parent->rotation_signs[i] < 0)
            result.translation[i]--;
    }
    return result;
}

struct model {
    size_t voxel_count;
    union { size_t memory_offset; fpos_t file_pos; };
};

struct node {
    uint32_t id;
    enum node_type { NODE_GROUP, NODE_SHAPE, NODE_TRANSFORM } type;
    unsigned height;
    union {
        struct {
            size_t model_idx;
        } shape;
        struct {
            size_t child_node_idx;
            size_t layer_idx;
            bool has_layer;
            bool is_hidden;
            struct transform transform;
        } transform;
        struct {
            size_t children_start, children_end; // range in group_children
        } group;
    };
};

struct layer {
    uint32_t id;
    bool is_hidden;
};

struct VxfFile {
    struct source {
        enum { SOURCE_MEMORY, SOURCE_FILE } type;
        union {
            struct { const char *buffer; size_t size, offset; } memory;
            struct { FILE *stream; bool from_filename; } file;
        };
    } source;
    Array(struct model) models;
    Array(struct model_size) model_sizes;
    Array(struct node) nodes;
    Array(size_t) group_children_node_idx;
    Array(struct layer) layers;
    const uint8_t (*palette)[4]; // either palette_buffer or default_palette
    uint8_t (*palette_buffer)[4];
    size_t readcounter;
    char tmpbuffer[GET_BYTES_MAX];
    struct {
        struct readstate_frame {
            size_t node_idx;
            size_t pos;
            struct transform transform;
        } *stack;
        size_t depth;
        VxfError error;
        bool eof;
    } readstate;
    struct retjmp retjmp;
};

// return next bytes, either directly from source memory or read into tmpbuffer
static const char *try_get_bytes(VxfFile *vf, size_t count) {
    assert(count <= GET_BYTES_MAX);
    struct source *src = &vf->source;
    vf->readcounter += count;
    switch (src->type) {
        case SOURCE_MEMORY:
            if (count > src->memory.size - src->memory.offset)
                return NULL;
            const char *result = &src->memory.buffer[src->memory.offset];
            src->memory.offset += count;
            return result;
        case SOURCE_FILE: {
            size_t read = fread(vf->tmpbuffer, count, 1, src->file.stream);
            if (read < 1 && ferror(src->file.stream))
                return_error(&vf->retjmp, VXF_ERROR_FILE_READ);
            if (read < 1)
                return NULL;
            return vf->tmpbuffer;
        }
        default: unreachable();
    }
}

static const void *get_bytes(VxfFile *vf, size_t count) {
    const char *result = try_get_bytes(vf, count);
    if (result) return result;
    return_error(&vf->retjmp, VXF_ERROR_UNEXPECTED_EOF);
}

static void skip_bytes(VxfFile *vf, size_t count) {
    struct source *src = &vf->source;
    vf->readcounter += count;
    switch (src->type) {
        case SOURCE_MEMORY:
            src->memory.offset += MIN(count, src->memory.size - src->memory.offset);
            break;
        case SOURCE_FILE:
            while (count > 0) {
                long offset = MIN(count, LONG_MAX);
                int seekerr = fseek(src->file.stream, offset, SEEK_CUR);
                if (seekerr) return_error(&vf->retjmp, VXF_ERROR_FILE_SEEK);
                count -= offset;
            }
            break;
    }
}

static const char *get_string(VxfFile *vf) {
    size_t rawlen = load_u32(get_bytes(vf, 4));
    size_t resultlen = MIN(rawlen, GET_BYTES_MAX - 1);
    const char *ro = get_bytes(vf, resultlen);
    skip_bytes(vf, rawlen - resultlen);
    if (ro != vf->tmpbuffer)
        memcpy(vf->tmpbuffer, ro, resultlen);
    vf->tmpbuffer[resultlen] = '\0';
    return vf->tmpbuffer;
}

static void skip_string(VxfFile *vf) {
    size_t length = load_u32(get_bytes(vf, 4));
    skip_bytes(vf, length);
}

static void skip_dict(VxfFile *vf) {
    uint32_t count = load_u32(get_bytes(vf, 4));
    while (count-- > 0) {
        skip_string(vf); // key
        skip_string(vf); // value
    }
}

static void parse_size_chunk(VxfFile *vf) {
    const char *data = get_bytes(vf, 12);
    *ARRAY_APPEND(vf->model_sizes, vf->retjmp) = (struct model_size){
        .size = {load_u32(data), load_u32(data + 4), load_u32(data + 8)}
    };
}

static void parse_model_chunk(VxfFile *vf) {
    size_t voxel_count = load_u32(get_bytes(vf, 4));
    struct model *model = ARRAY_APPEND(vf->models, vf->retjmp);
    *model = (struct model){.voxel_count = voxel_count};
    const struct source *src = &vf->source;
    switch (src->type) {
        case SOURCE_MEMORY:
            model->memory_offset = src->memory.offset;
            break;
        case SOURCE_FILE:
            if (fgetpos(src->file.stream, &model->file_pos) != 0)
                return_error(&vf->retjmp, VXF_ERROR_FILE_SEEK);
            break;
    }
    skip_bytes(vf, 4 * voxel_count);
}

static void seek_to_model(VxfFile *vf, const struct model *model) {
    switch (vf->source.type) {
        case SOURCE_MEMORY:
            vf->source.memory.offset = model->memory_offset;
            break;
        case SOURCE_FILE:
            if (fsetpos(vf->source.file.stream, &model->file_pos) != 0)
                return_error(&vf->retjmp, VXF_ERROR_FILE_SEEK);
            break;
    }
}

static void parse_shape_chunk(VxfFile *vf) {
    uint32_t node_id = load_u32(get_bytes(vf, 4));
    struct node *node = ARRAY_APPEND(vf->nodes, vf->retjmp);
    *node = (struct node){.id = node_id, .type = NODE_SHAPE, .shape = {0}};
    skip_dict(vf);
    uint32_t model_count = load_u32(get_bytes(vf, 4));
    if (model_count < 1) return_error(&vf->retjmp, VXF_ERROR_INVALID_FILE_STRUCTURE);
    for (uint32_t i = 0; i < model_count; i++) {
        size_t model_index = load_u32(get_bytes(vf, 4));
        if (i == 0) node->shape.model_idx = model_index; // other animation frames ignored for now
        skip_dict(vf);
    }
}

static void parse_group_chunk(VxfFile *vf) {
    uint32_t node_id = load_u32(get_bytes(vf, 4));
    struct node *node = ARRAY_APPEND(vf->nodes, vf->retjmp);
    *node = (struct node){.id = node_id, .type = NODE_GROUP, .group = {0}};
    skip_dict(vf);
    uint32_t child_count = load_u32(get_bytes(vf, 4));
    node->group.children_start = vf->group_children_node_idx.len;
    for (uint32_t i = 0; i < child_count; i++)
        *ARRAY_APPEND(vf->group_children_node_idx, vf->retjmp) = load_u32(get_bytes(vf, 4));
    node->group.children_end = vf->group_children_node_idx.len;
}

static bool parse_is_hidden_dict(VxfFile *vf) {
    bool is_hidden = false;
    uint32_t entry_count = load_u32(get_bytes(vf, 4));
    while (entry_count-- > 0) {
        const char *key = get_string(vf);
        if (strcmp(key, "_hidden") == 0) {
            int value;
            sscanf(get_string(vf), "%d", &value);
            is_hidden = value;
        } else {
            skip_string(vf);
        }
    }
    return is_hidden;
}

static struct transform parse_transform_frame_dict(VxfFile *vf) {
    struct transform t = TRANSFORM_IDENTITY;
    uint32_t entry_count = load_u32(get_bytes(vf, 4));
    while (entry_count-- > 0) {
        const char *key = get_string(vf);
        if (strcmp(key, "_r") == 0) {
            unsigned rotcode;
            sscanf(get_string(vf), "%u", &rotcode);
            t.rotation_cols[0] = MIN(2u, rotcode & 0x3);
            t.rotation_cols[1] = MIN(2u, rotcode >> 2 & 0x3);
            t.rotation_cols[2] = MIN(2u, 3u - t.rotation_cols[0] - t.rotation_cols[1]);
            t.rotation_signs[0] = rotcode & 0x10 ? -1 : 1;
            t.rotation_signs[1] = rotcode & 0x20 ? -1 : 1;
            t.rotation_signs[2] = rotcode & 0x40 ? -1 : 1;
        } else if (strcmp(key, "_t") == 0) {
            sscanf(get_string(vf), "%"SCNi32"%"SCNi32"%"SCNi32"",
                &t.translation[0], &t.translation[1], &t.translation[2]);
        } else {
            skip_string(vf);
        }
    }
    return t;
}

static void parse_transform_chunk(VxfFile *vf) {
    uint32_t node_id = load_u32(get_bytes(vf, 4));
    struct node *node = ARRAY_APPEND(vf->nodes, vf->retjmp);
    *node = (struct node){.id = node_id, .type = NODE_TRANSFORM, .transform = {0}};

    node->transform.is_hidden = parse_is_hidden_dict(vf);
    const char *data = get_bytes(vf, 16);

    node->transform.child_node_idx = load_u32(data);

    int32_t layervalue = load_i32(data + 8);
    node->transform.has_layer = layervalue >= 0;
    node->transform.layer_idx = MAX(layervalue, 0);

    uint32_t frame_count = load_u32(data + 12);
    if (frame_count < 1) return_error(&vf->retjmp, VXF_ERROR_INVALID_FILE_STRUCTURE);
    for (uint32_t i = 0; i < frame_count; i++) {
        if (i == 0) node->transform.transform = parse_transform_frame_dict(vf);
        else skip_dict(vf);
    }
}

static void parse_layer_chunk(VxfFile *vf) {
    uint32_t layer_id = load_u32(get_bytes(vf, 4));
    struct layer *layer = ARRAY_APPEND(vf->layers, vf->retjmp);
    *layer = (struct layer){.id = layer_id, .is_hidden = parse_is_hidden_dict(vf)};
    skip_bytes(vf, 4);
}

static void parse_rgba_chunk(VxfFile *vf) {
    const void *data = get_bytes(vf, 256 * 4);
    assert(!vf->palette_buffer);
    vf->palette_buffer = xcalloc(256, 4, &vf->retjmp);
    // shift because the palette starts at index 1; leave index 0 as zero
    memcpy(vf->palette_buffer + 1, data, 255 * sizeof *vf->palette_buffer);
    vf->palette = (const uint8_t(*)[4])vf->palette_buffer;
}

static void parse_main_children(VxfFile *vf) {
    // we ignore the declared size and just read until eof, for better compatibility and avoiding the 2/4 gb limit
    for (const char *header; (header = try_get_bytes(vf, 12));) {
        uint32_t fourcc = load_u32(header);
        size_t contentsize = load_u32(header + 4);
        size_t childrensize = load_u32(header + 8);

        vf->readcounter = 0;
        switch (fourcc) {
            case FOURCC_SIZE: parse_size_chunk(vf); break;
            case FOURCC_XYZI: parse_model_chunk(vf); break;
            case FOURCC_RGBA: parse_rgba_chunk(vf); break;
            case FOURCC_nSHP: parse_shape_chunk(vf); break;
            case FOURCC_nGRP: parse_group_chunk(vf); break;
            case FOURCC_nTRN: parse_transform_chunk(vf); break;
            case FOURCC_LAYR: parse_layer_chunk(vf); break;
            default: skip_bytes(vf, contentsize);
        }
        if (vf->readcounter != contentsize)
            return_error(&vf->retjmp, VXF_ERROR_INVALID_FILE_STRUCTURE);
        skip_bytes(vf, childrensize);
    }
}

static void parse_vox(VxfFile *vf) {
    const char *header = get_bytes(vf, 20);
    uint32_t vox_fourcc = load_u32(header + 0);
    uint32_t main_fourcc = load_u32(header + 8);
    size_t contentsize = load_u32(header + 12);
    if (vox_fourcc != FOURCC_VOX || main_fourcc != FOURCC_MAIN)
        return_error(&vf->retjmp, VXF_ERROR_UNRECOGNIZED_FILE_FORMAT);
    skip_bytes(vf, contentsize);
    parse_main_children(vf);
}

static int cmp_u32(const void *p1, const void *p2) {
    const uint32_t x1 = *(uint32_t*)p1, x2 = *(uint32_t*)p2;
    return (x1 > x2) - (x1 < x2);
}

static size_t get_node_index_by_id(VxfFile *vf, uint32_t id) {
    static_assert(offsetof(struct node, id) == 0, "node struct starts with id");
    struct node *node = bsearch(&id, vf->nodes.items, vf->nodes.len, sizeof *vf->nodes.items, cmp_u32);
    if (node == NULL) return_error(&vf->retjmp, VXF_ERROR_INVALID_SCENE);
    return node - vf->nodes.items;
}

static size_t get_layer_index_by_id(VxfFile *vf, uint32_t id) {
    static_assert(offsetof(struct layer, id) == 0, "layer struct starts with id");
    struct layer *layer = bsearch(&id, vf->layers.items, vf->layers.len, sizeof *vf->layers.items, cmp_u32);
    if (layer == NULL) return_error(&vf->retjmp, VXF_ERROR_INVALID_SCENE);
    return layer - vf->layers.items;
}

// node and layers IDs could theoretically be sparse and unordered in the file; we replace the
// raw IDs read from the vox file with array indices here.
static void replace_ids(VxfFile *vf) {
    if (vf->nodes.len > 1) {
        qsort(vf->nodes.items, vf->nodes.len, sizeof *vf->nodes.items, cmp_u32);
    }
    if (vf->layers.len > 1) {
        qsort(vf->layers.items, vf->layers.len, sizeof *vf->layers.items, cmp_u32);
    }
    for (size_t i = 0; i < vf->group_children_node_idx.len; i++) {
        vf->group_children_node_idx.items[i] = get_node_index_by_id(vf, vf->group_children_node_idx.items[i]);
    }
    for (size_t i = 0; i < vf->nodes.len; i++) {
        struct node *node = &vf->nodes.items[i];
        if (node->type == NODE_TRANSFORM) {
            node->transform.child_node_idx = get_node_index_by_id(vf, node->transform.child_node_idx);
            if (node->transform.has_layer) {
                node->transform.layer_idx = get_layer_index_by_id(vf, node->transform.layer_idx);
            }
        }
    }
}

// assigns node heights and checks for cycles; assumes heights initialized to 0
static unsigned check_scene_tree_recursive(VxfFile *vf, uint32_t node_id) {
    struct node *node = &vf->nodes.items[node_id];
    if (node->height == UINT_MAX)
        goto invalid_scene;
    if (node->height > 0)
        return node->height; // already processed
    node->height = UINT_MAX; // set temporarily to detect cycles

    switch (node->type) {
        case NODE_SHAPE:
            if (node->shape.model_idx >= vf->models.len)
                return_error(&vf->retjmp, VXF_ERROR_INVALID_SCENE);
            return node->height = 0;
        case NODE_TRANSFORM: {
            uint32_t child_height = check_scene_tree_recursive(vf, node->transform.child_node_idx);
            if (child_height >= UINT_MAX) goto invalid_scene;
            return node->height = child_height + 1;
        }
        case NODE_GROUP: {
            if (node->group.children_start == node->group.children_end)
                return node->height = 0;

            uint32_t max_child_height = 0;
            for (size_t i = node->group.children_start; i < node->group.children_end; i++) {
                uint32_t child_height = check_scene_tree_recursive(vf, vf->group_children_node_idx.items[i]);
                max_child_height = MAX(max_child_height, child_height);
            }
            if (max_child_height >= UINT_MAX) goto invalid_scene;
            return node->height = max_child_height + 1;
        }
    }
invalid_scene:
    return_error(&vf->retjmp, VXF_ERROR_INVALID_SCENE);
}

static bool open_common(VxfFile *vf, VxfError *error) {
    if (setjmp(vf->retjmp.jump)) { // handle error
        assert(vf->retjmp.error != VXF_SUCCESS);
        if (error) *error = vf->retjmp.error;
        return false;
    }

    vf->palette = default_palette;
    parse_vox(vf);

    if (vf->models.len == 0 || vf->models.len != vf->model_sizes.len)
        return_error(&vf->retjmp, VXF_ERROR_INVALID_SCENE);

    // create a root node for single-model files without a scene graph
    if (vf->nodes.len == 0) {
        *ARRAY_APPEND(vf->nodes, vf->retjmp) = (struct node){
            .id = 0, .type = NODE_SHAPE, .shape = {.model_idx = 0}
        };
    }

    replace_ids(vf);
    check_scene_tree_recursive(vf, 0);
    if (error) *error = VXF_SUCCESS;
    return true;
}

VxfFile *vxf_open_file(const char *filename, VxfError *error) {
    VxfFile *vf = malloc(sizeof *vf);
    if (!vf) {
        if (error) *error = VXF_ERROR_OUT_OF_MEMORY;
        return NULL;
    }
    FILE *stream = fopen(filename, "rb");
    if (!stream) {
        if (error) *error = VXF_ERROR_FILE_OPEN;
        free(vf);
        return NULL;
    }
    *vf = (VxfFile){
        .source = {.type = SOURCE_FILE, .file = {.stream = stream, .from_filename = true}},
    };
    if (!open_common(vf, error)) {
        vxf_close(vf);
        return NULL;
    }
    return vf;
}

VxfFile *vxf_open_stream(FILE *stream, VxfError *error) {
    clearerr(stream);
    VxfFile *vf = malloc(sizeof *vf);
    if (!vf) {
        if (error) *error = VXF_ERROR_OUT_OF_MEMORY;
        return NULL;
    }
    *vf = (VxfFile){.source = {.type = SOURCE_FILE, .file.stream = stream}};
    if (!open_common(vf, error)) {
        vxf_close(vf);
        return NULL;
    }
    return vf;
}

VxfFile *vxf_open_memory(size_t size, const char buffer[], VxfError *error) {
    VxfFile *vf = malloc(sizeof *vf);
    if (!vf) {
        if (error) *error = VXF_ERROR_OUT_OF_MEMORY;
        return NULL;
    }
    *vf = (VxfFile){
        .source = {.type = SOURCE_MEMORY, .memory = {.size = size, .buffer = buffer}}
    };
    if (!open_common(vf, error)) {
        vxf_close(vf);
        return NULL;
    }
    return vf;
}

static void extend_bounds(int32_t xyzmin[3], int32_t xyzmax[3], const struct transform *transform, const uint8_t modelpos[3]) {
    int32_t globalpos[3];
    apply_transform(transform, modelpos, globalpos);
    for (size_t i = 0; i < 3; i++) {
        xyzmin[i] = MIN(xyzmin[i], globalpos[i]);
        xyzmax[i] = MAX(xyzmax[i], globalpos[i]);
    }
}

static void extend_bounds_recursive(const VxfFile *vf, int32_t xyzmin[3], int32_t xyzmax[3], const struct transform *parent, uint32_t node_id) {
    struct node *node = &vf->nodes.items[node_id];
    switch (node->type) {
        case NODE_SHAPE: {
            const struct model_size *size = &vf->model_sizes.items[node->shape.model_idx];
            struct transform transform = get_model_transform(parent, size);
            extend_bounds(xyzmin, xyzmax, &transform, (uint8_t[3]){0, 0, 0});
            extend_bounds(xyzmin, xyzmax, &transform, (uint8_t[3]){
                CLAMP(size->size[0], 1, 256) - 1,
                CLAMP(size->size[1], 1, 256) - 1,
                CLAMP(size->size[2], 1, 256) - 1,
            });
            break;
        }
        case NODE_TRANSFORM: {
            if (node->transform.is_hidden || (node->transform.has_layer && vf->layers.items[node->transform.layer_idx].is_hidden))
                break;
            struct transform transform = combine_transforms(parent, &node->transform.transform);
            extend_bounds_recursive(vf, xyzmin, xyzmax, &transform, node->transform.child_node_idx);
            break;
        }
        case NODE_GROUP:
            for (size_t i = node->group.children_start; i < node->group.children_end; i++)
                extend_bounds_recursive(vf, xyzmin, xyzmax, parent, vf->group_children_node_idx.items[i]);
            break;
    }
}

void vxf_calculate_bounds(const VxfFile* vf, int32_t xyz_min[3], int32_t xyz_max[3]) {
    for (int i = 0; i < 3; i++) {
        xyz_min[i] = INT32_MAX, xyz_max[i] = INT32_MIN;
    }
    extend_bounds_recursive(vf, xyz_min, xyz_max, &TRANSFORM_IDENTITY, 0);

    // no voxels found, reset to 0
    if (xyz_min[0] > xyz_max[0]) {
        for (int i = 0; i < 3; i++)
            xyz_min[i] = xyz_max[i] = 0;
    }
}

static uintmax_t count_voxels_recursive(const VxfFile *vf, uint32_t node_id) {
    struct node *node = &vf->nodes.items[node_id];
    switch (node->type) {
        case NODE_SHAPE:
            return vf->models.items[node->shape.model_idx].voxel_count;
        case NODE_TRANSFORM:
            if (node->transform.is_hidden || (node->transform.has_layer && vf->layers.items[node->transform.layer_idx].is_hidden))
                return 0;
            return count_voxels_recursive(vf, node->transform.child_node_idx);
        case NODE_GROUP: {
            uintmax_t sum = 0;
            for (size_t i = node->group.children_start; i < node->group.children_end; i++)
                sum += count_voxels_recursive(vf, vf->group_children_node_idx.items[i]);
            return sum;
        }
        default: unreachable();
    }
}

uintmax_t vxf_count_voxels(const VxfFile* vf) {
    return count_voxels_recursive(vf, 0);
}

void vxf_get_palette(const VxfFile *vf, uint8_t rgba_buf[256][4]) {
    const uint8_t (*palette)[4] = vf ? vf->palette : default_palette;
    memcpy(rgba_buf, palette, 256 * sizeof *palette);
}

static struct readstate_frame start_frame(VxfFile *vf, size_t node_idx, const struct transform *parent_transform) {
    const struct node *node = &vf->nodes.items[node_idx];
    switch (node->type) {
        case NODE_SHAPE: {
            const struct model *model = &vf->models.items[node->shape.model_idx];
            const struct model_size *size = &vf->model_sizes.items[node->shape.model_idx];
            seek_to_model(vf, model);
            return (struct readstate_frame){
                .node_idx = node_idx,
                .transform = get_model_transform(parent_transform, size),
            };
        }
        case NODE_TRANSFORM:
            return (struct readstate_frame){
                .node_idx = node_idx,
                .transform = combine_transforms(parent_transform, &node->transform.transform)
            };
        case NODE_GROUP:
            return (struct readstate_frame){.node_idx = node_idx, .transform = *parent_transform};
        default: unreachable();
    }
}

struct readbuffers {
    int32_t (*xyz)[3];
    uint8_t (*rgba)[4];
    uint8_t *coloridx;
    size_t max_count;
};

static void read_model_voxels(VxfFile *vf, const struct transform *transform, const struct readbuffers *buffers, size_t offset, size_t count) {
    assert(count <= buffers->max_count);
    while (count > 0) {
        size_t n = MIN(count, GET_BYTES_MAX / 4);
        const uint8_t (*xyzidata)[4] = (const uint8_t(*)[4])get_bytes(vf, n * 4);
        for (size_t i = 0; i < n; i++) {
            const uint8_t *modelpos = xyzidata[i];
            apply_transform(transform, modelpos, buffers->xyz[offset + i]);
        }
        if (buffers->rgba) {
            for (size_t i = 0; i < n; i++) {
                uint8_t coloridx = xyzidata[i][3];
                const uint8_t (*color)[4] = &vf->palette[coloridx];
                memcpy(&buffers->rgba[offset + i], color, sizeof *color);
            }
        }
        if (buffers->coloridx) {
            for (size_t i = 0; i < n; i++) {
                uint8_t coloridx = xyzidata[i][3];
                buffers->coloridx[offset + i] = coloridx;
            }
        }
        offset += n, count -= n;
    }
}

static enum { CONTINUE_FRAME_COMPLETE, CONTINUE_FRAME_INCOMPLETE, CONTINUE_FRAME_CHILD }
continue_frame(VxfFile *vf, struct readstate_frame *frame, const struct readbuffers *buffers, size_t *count_read, size_t *child_node_idx) {
    const struct node *node = &vf->nodes.items[frame->node_idx];
    switch (node->type) {
        case NODE_SHAPE: {
            const struct model *model = &vf->models.items[node->shape.model_idx];
            size_t count = MIN(buffers->max_count - *count_read, model->voxel_count - frame->pos);
            read_model_voxels(vf, &frame->transform, buffers, *count_read, count);
            frame->pos += count;
            *count_read += count;
            return frame->pos < model->voxel_count ?
                CONTINUE_FRAME_INCOMPLETE : CONTINUE_FRAME_COMPLETE;
        } case NODE_TRANSFORM:
            if (frame->pos++ > 0)
                return CONTINUE_FRAME_COMPLETE;
            if (node->transform.is_hidden || (node->transform.has_layer && vf->layers.items[node->transform.layer_idx].is_hidden))
                return CONTINUE_FRAME_COMPLETE;
            *child_node_idx = node->transform.child_node_idx;
            return CONTINUE_FRAME_CHILD;
        case NODE_GROUP:
            if (node->group.children_start + frame->pos >= node->group.children_end)
                return CONTINUE_FRAME_COMPLETE;
            *child_node_idx = vf->group_children_node_idx.items[node->group.children_start + frame->pos++];
            return CONTINUE_FRAME_CHILD;
        default: unreachable();
    }
}

static size_t read_common(VxfFile *vf, const struct readbuffers *buffers, VxfError *error) {
    if (vf->readstate.eof) {
        if (error) *error = VXF_SUCCESS;
        return 0;
    }
    if (setjmp(vf->retjmp.jump)) {
        assert(vf->retjmp.error != VXF_SUCCESS);
        vf->readstate.error = vf->retjmp.error;
    }
    if (vf->readstate.error) {
        if (error) *error = vf->readstate.error;
        return 0;
    }
    if (!vf->readstate.stack) {
        vf->readstate.stack = xcalloc(vf->nodes.items[0].height + 1, sizeof *vf->readstate.stack, &vf->retjmp);
        vf->readstate.stack[0] = start_frame(vf, 0, &TRANSFORM_IDENTITY);
    }

    size_t count_read = 0;
    while (count_read < buffers->max_count) {
        size_t child_node_idx;
        switch (continue_frame(vf, &vf->readstate.stack[vf->readstate.depth], buffers, &count_read, &child_node_idx)) {
            case CONTINUE_FRAME_INCOMPLETE:
                assert(count_read == buffers->max_count);
                break;
            case CONTINUE_FRAME_COMPLETE:
                if (vf->readstate.depth == 0) {
                    vf->readstate.eof = true;
                    goto finish;
                }
                vf->readstate.depth--;
                break;
            case CONTINUE_FRAME_CHILD: {
                struct transform *parent_transform = &vf->readstate.stack[vf->readstate.depth].transform;
                vf->readstate.stack[++vf->readstate.depth] = start_frame(vf, child_node_idx, parent_transform);
                break;
            }
        }
    }
finish:
    if (error) *error = VXF_SUCCESS;
    return count_read;
}

size_t vxf_read_xyz_rgba(VxfFile *vf, size_t max_count, int32_t xyz_buf[][3], uint8_t rgba_buf[][4], VxfError *error) {
    if (!vf || ((!xyz_buf || !rgba_buf)  && max_count > 0)) {
        if (error) *error = VXF_ERROR_INVALID_ARGUMENT;
        return 0;
    }
    return read_common(vf, &(struct readbuffers){
        .xyz = xyz_buf,
        .rgba = rgba_buf,
        .max_count = max_count,
    }, error);
}

size_t vxf_read_xyz_coloridx(VxfFile *vf, size_t max_count, int32_t xyz_buf[][3], uint8_t coloridx_buf[], VxfError *error) {
    if (!vf || ((!xyz_buf || !coloridx_buf)  && max_count > 0)) {
        if (error) *error = VXF_ERROR_INVALID_ARGUMENT;
        return 0;
    }
    return read_common(vf, &(struct readbuffers){
        .xyz = xyz_buf,
        .coloridx = coloridx_buf,
        .max_count = max_count,
    }, error);
}

void vxf_close(VxfFile *vf) {
    if (!vf) return;
    if (vf->source.type == SOURCE_FILE && vf->source.file.from_filename)
        fclose(vf->source.file.stream);
    free(vf->nodes.items);
    free(vf->models.items);
    free(vf->model_sizes.items);
    free(vf->group_children_node_idx.items);
    free(vf->layers.items);
    free(vf->readstate.stack);
    free(vf->palette_buffer);
    free(vf);
}

const char *vxf_error_string(VxfError error) {
    switch (error) {
        case VXF_SUCCESS: return "Operation successful";
        case VXF_ERROR_FILE_OPEN: return "Failed to open input file";
        case VXF_ERROR_FILE_READ: return "Failed to read from input stream";
        case VXF_ERROR_FILE_SEEK: return "Input stream is not seekable";
        case VXF_ERROR_UNRECOGNIZED_FILE_FORMAT: return "Unrecognized file format";
        case VXF_ERROR_UNEXPECTED_EOF: return "Unexpected end of input";
        case VXF_ERROR_INVALID_FILE_STRUCTURE: return "Invalid vox file structure";
        case VXF_ERROR_INVALID_SCENE: return "Invalid scene graph";
        case VXF_ERROR_OUT_OF_MEMORY: return "Out of memory or size overflow";
        case VXF_ERROR_INVALID_ARGUMENT: return "Invalid argument provided";
        default: return "Unmapped error";
    }
}

