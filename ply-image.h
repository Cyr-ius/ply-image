#include <stdint.h>
#include <stdbool.h>
#include <png.h>

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MAX(a, b) ((a) >= (b) ? (a) : (b))

#define CLAMP(a, b, c) (MIN(MAX((a), (b)), (c)))

typedef union {
    uint32_t *as_pixels;
    png_byte *as_png_bytes;
    char *address;
} ply_image_layout_t;

typedef struct _ply_image
{
    char *filename;
    FILE *fp;
    ply_image_layout_t layout;
    size_t size;
    long width;
    long height;
} ply_image_t;

static bool ply_image_open_file(ply_image_t *image);
static void ply_image_close_file(ply_image_t *image);
ply_image_t *ply_image_new(const char *filename);
void ply_image_free(ply_image_t *image);
uint32_t *ply_image_get_data(ply_image_t *image);
long ply_image_get_width(ply_image_t *image);
long ply_image_get_height(ply_image_t *image);
ply_image_t *
ply_image_resize(ply_image_t *image,
                 long width,
                 long height);
static void transform_to_argb32(png_struct *png,
                                png_row_info *row_info,
                                png_byte *data);
static void transform_to_rgb32(png_struct *png,
                               png_row_info *row_info,
                               png_byte *data);
bool ply_image_load(ply_image_t *image);
uint32_t *ply_image_get_data(ply_image_t *image);
static ssize_t ply_image_get_size(ply_image_t *image);
long ply_image_get_width(ply_image_t *image);
long ply_image_get_height(ply_image_t *image);
ply_image_t *ply_image_resize(ply_image_t *image,
                              long width,
                              long height);
ply_image_t *ply_image_rotate(ply_image_t *image,
                              long center_x,
                              long center_y,
                              double theta_offset);
