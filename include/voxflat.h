#ifndef VOXFLAT_H
#define VOXFLAT_H
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque struct representing an opened MagicaVoxel vox file.
 *
 * Allocated by @ref vxf_open_file, @ref vxf_open_stream or @ref vxf_open_memory.
 * Has to be freed by calling @ref vxf_close.
 */
typedef struct VxfFile VxfFile;

/**
 * @brief Error codes for open and read functions.
 */
typedef enum {
    VXF_SUCCESS = 0,                    /**< Operation successful. */
    VXF_ERROR_FILE_OPEN = 1,            /**< Failed to open input file. */
    VXF_ERROR_FILE_READ = 2,            /**< Failed to read from input stream. */
    VXF_ERROR_FILE_SEEK = 3,            /**< Input stream is not seekable. */
    VXF_ERROR_UNRECOGNIZED_FILE_FORMAT = 4, /**< Unrecognized file format. */
    VXF_ERROR_UNEXPECTED_EOF = 5,       /**< Unexpected end of input. */
    VXF_ERROR_INVALID_FILE_STRUCTURE = 6, /**< Invalid vox file structure. */
    VXF_ERROR_INVALID_SCENE = 7,        /**< Invalid scene graph. */
    VXF_ERROR_OUT_OF_MEMORY = 8,        /**< Out of memory or size overflow. */
    VXF_ERROR_INVALID_ARGUMENT = 9,     /**< Invalid argument provided. */
} VxfError;

/**
 * @brief Opens a MagicaVoxel vox file from a filename.
 *
 * The file must be seekable and will be kept open until @ref vxf_close is called.
 *
 * @param[in] filename Path to the vox file.
 * @param[out] error Where to store the error code. May be NULL.
 *
 * @return Pointer to a new VxfFile instance on success, NULL on failure.
 */
VxfFile *vxf_open_file(const char *filename, VxfError *error);

/**
 * @brief Opens a MagicaVoxel vox file from a stdio file stream.
 *
 * The stream must be a seekable binary stream (e.g. openend with `fopen(..., "rb")`), ist must remain open
 * and must ust be used from outside the library until @ref vxf_close is called.
 *
 * @param[in] stream Stream to read from.
 * @param[out] error Where to store the error code. May be NULL.
 *
 * @return Pointer to a new VxfFile instance on success, NULL on failure.
 */
VxfFile *vxf_open_stream(FILE *stream, VxfError *error);

/**
 * @brief Opens a MagicaVoxel vox file from a memory buffer.
 *
 * The buffer must remain accessible until @ref vxf_close is called.
 *
 * @param[in] size Size of the buffer.
 * @param[in] buffer Memory buffer containing vox file data.
 * @param[out] error Where to store the error code. May be NULL.
 *
 * @return Pointer to a new VxfFile instance on success, NULL on failure.
 */
VxfFile *vxf_open_memory(size_t size, const char buffer[], VxfError *error);

/**
 * @brief Calculates the bounding box of the voxel data.
 *
 * Calculate minimum and maximum values of voxel coordinates based in the model
 * sizes and transforms of the vox scene.
 *
 * @param[in] vf VxfFile instance.
 * @param[out] xyz_min Minimum x, y, z coordinates of the bounding box.
 * @param[out] xyz_max Maximum x, y, z coordinates of the bounding box.
 */
void vxf_calculate_bounds(const VxfFile* vf, int32_t xyz_min[3], int32_t xyz_max[3]);

/**
 * @brief Counts the total number of voxels in the file.
 *
 * The result should be equal to the number of voxels returned by repeated calls to the
 * @ref vxf_read_xyz_rgba and @ref vxf_read_xyz_coloridx functions.*
 *
 * @param[in] vf VxfFile instance.
 * @return Total number of voxels.
 */
uintmax_t vxf_count_voxels(const VxfFile* vf);

/**
 * @brief Retrieves the color palette.
 *
 * If the vox file does not contain a palette (RGBA) chunk, the default palette
 * is returned. The actual palette entries start at index 1 like in MagicaVoxel;
 * color 0 is always set to {0,0,0,0}.
 *
 * @param[in] vf VxfFile instance, or NULL to return the default palette.
 * @param[out] rgba_buf Buffer to store 256 RGBA color entries (256 * 4 bytes).
 */
void vxf_get_palette(const VxfFile *vf, uint8_t rgba_buf[256][4]);

/**
 * @brief Reads voxel positions and RGBA colors.
 *
 * Can be called repeatedly to read all the voxels of the vox scene, i.e. voxels from all model instances of
 * the scene. The returned coordinates are in the global coordinate system. Model instances that are hidden
 * or assigned to hidden layers are not returned. The same Voxels may be returned multiple times with possibly
 * different coordinates if there are multiple instances of a model in the scene.
 *
 * The passed buffers must be large enough for `max_count` voxels.
 *
 * @param[in] vf VxfFile instance.
 * @param[in] max_count Maximum number of voxels to read.
 * @param[out] xyz_buf Buffer for voxel x, y, z coordinates.
 * @param[out] rgba_buf Buffer for voxel RGBA colors.
 * @param[out] error Where to store the error code. May be NULL.
 *
 * @return Number of voxels read into the buffers, which can be less than `max_count` or 0 if the
 * end of the list of available voxels has been reached; 0 if an error has occurred.
 */
size_t vxf_read_xyz_rgba(VxfFile *vf, size_t max_count, int32_t xyz_buf[][3], uint8_t rgba_buf[][4], VxfError *error);

/**
 * @brief Reads voxel positions and color indices.
 *
 * Like @ref vxf_read_xyz_rgba, but instead of directly returning the RGBA colors, returns the palette color index
 * for each voxelof each voxel's color. The color indices are valid indices for the palette returned by
 * @ref vxf_get_palette. Color indices should usually be in the 1 to 255 range, but might be 0 values can
 * theoretically be stored.
 *
 * @param[in] vf VxfFile instance.
 * @param[in] max_count Maximum number of voxels to read.
 * @param[out] xyz_buf Buffer for voxel x, y, z coordinates.
 * @param[out] coloridx_buf Buffer for voxel color indices.
 * @param[out] error Where to store the error code. May be NULL.
 *
 * @return Number of voxels read into the buffers, which can be less than `max_count` or 0 if the
 * end of the list of available voxels has been reached; 0 if an error has occurred.
 */
size_t vxf_read_xyz_coloridx(VxfFile *vf, size_t max_count, int32_t xyz_buf[][3], uint8_t coloridx_buf[], VxfError *error);

/**
 * @brief Destroys a VxfFile instance.
 *
 * If the instance was created via @ref vxf_open_file, the file is closed.
 * If the instance was created via @ref vxf_open_file, the passed stream is *not* closed.
 *
 * @param[in] vf VxfFile instance.
 */
void vxf_close(VxfFile *vf);

/**
 * @brief Converts an error code to a human-readable string.
 * @param[in] error Error code to convert.
 * @return Static string describing the error.
 */
const char *vxf_error_string(VxfError error);

#ifdef __cplusplus
}
#endif
#endif // VOXFLAT_H
