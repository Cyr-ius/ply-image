/* ply-image.c - png file loader
 *
 * Copyright (C) 2006, 2007 Red Hat, Inc.
 * Copyright (C) 2003 University of Southern California
 * Copyright (c) 2014 Sam Nazarko
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Some implementation taken from the cairo library.
 *
 * Written by: Charlie Brej <cbrej@cs.man.ac.uk>
 *             Kristian Høgsberg <krh@redhat.com>
 *             Ray Strode <rstrode@redhat.com>
 *             Carl D. Worth <cworth@cworth.org>
 *             Cedric Levasseur <cedric.levasseur@ipocus.net>
 */
#include "plymouth-lite.h"
#include "ply-image.h"

static bool
ply_image_open_file(ply_image_t *image)
{
    assert(image != NULL);

    image->fp = fopen(image->filename, "r");

    if (image->fp == NULL)
        return false;
    return true;
}

static void
ply_image_close_file(ply_image_t *image)
{
    assert(image != NULL);

    if (image->fp == NULL)
        return;
    fclose(image->fp);
    image->fp = NULL;
}

ply_image_t *
ply_image_new(const char *filename)
{
    ply_image_t *image;

    assert(filename != NULL);

    image = calloc(1, sizeof(ply_image_t));

    image->filename = strdup(filename);
    image->fp = NULL;
    image->layout.address = NULL;
    image->size = -1;
    image->width = -1;
    image->height = -1;

    return image;
}

void ply_image_free(ply_image_t *image)
{
    if (image == NULL)
        return;

    assert(image->filename != NULL);

    if (image->layout.address != NULL)
    {
        free(image->layout.address);
        image->layout.address = NULL;
    }

    free(image->filename);
    free(image);
}

static void
transform_to_argb32(png_struct *png,
                    png_row_info *row_info,
                    png_byte *data)
{
    unsigned int i;

    for (i = 0; i < row_info->rowbytes; i += 4)
    {
        uint8_t red, green, blue, alpha;
        uint32_t pixel_value;

        red = data[i + 0];
        green = data[i + 1];
        blue = data[i + 2];
        alpha = data[i + 3];

        red = (uint8_t)CLAMP(((red / 255.0) * (alpha / 255.0)) * 255.0, 0, 255.0);
        green = (uint8_t)CLAMP(((green / 255.0) * (alpha / 255.0)) * 255.0,
                               0, 255.0);
        blue = (uint8_t)CLAMP(((blue / 255.0) * (alpha / 255.0)) * 255.0, 0, 255.0);

        pixel_value = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
        memcpy(data + i, &pixel_value, sizeof(uint32_t));
    }
}

static void
transform_to_rgb32(png_struct *png,
                   png_row_info *row_info,
                   png_byte *data)
{
    unsigned int i;

    for (i = 0; i < row_info->rowbytes; i += 4)
    {
        uint8_t red, green, blue, alpha;
        uint32_t pixel_value;

        red = data[i + 0];
        green = data[i + 1];
        blue = data[i + 2];
        alpha = data[i + 3];
        pixel_value = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
        memcpy(data + i, &pixel_value, sizeof(uint32_t));
    }
}

bool ply_image_load(ply_image_t *image)
{
    png_struct *png;
    png_info *info;
    png_uint_32 width, height, bytes_per_row, row;
    int bits_per_pixel, color_type, interlace_method;
    png_byte **rows;

    assert(image != NULL);

    if (!ply_image_open_file(image))
        return false;

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(png != NULL);

    info = png_create_info_struct(png);
    assert(info != NULL);

    png_init_io(png, image->fp);

    if (setjmp(png_jmpbuf(png)) != 0)
    {
        ply_image_close_file(image);
        return false;
    }

    png_read_info(png, info);
    png_get_IHDR(png, info,
                 &width, &height, &bits_per_pixel,
                 &color_type, &interlace_method, NULL, NULL);
    bytes_per_row = 4 * width;

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if ((color_type == PNG_COLOR_TYPE_GRAY) && (bits_per_pixel < 8))
        png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if (bits_per_pixel == 16)
        png_set_strip_16(png);

    if (bits_per_pixel < 8)
        png_set_packing(png);

    if ((color_type == PNG_COLOR_TYPE_GRAY) || (color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
        png_set_gray_to_rgb(png);

    if (interlace_method != PNG_INTERLACE_NONE)
        png_set_interlace_handling(png);

    png_set_filler(png, 0xff, PNG_FILLER_AFTER);

    png_set_read_user_transform_fn(png, transform_to_rgb32);

    png_read_update_info(png, info);

    rows = malloc(height * sizeof(png_byte *));
    image->layout.address = malloc(height * bytes_per_row);

    for (row = 0; row < height; row++)
        rows[row] = &image->layout.as_png_bytes[row * bytes_per_row];

    png_read_image(png, rows);

    free(rows);
    png_read_end(png, info);
    ply_image_close_file(image);
    png_destroy_read_struct(&png, &info, NULL);

    image->width = width;
    image->height = height;

    return true;
}

uint32_t *
ply_image_get_data(ply_image_t *image)
{
    assert(image != NULL);

    return image->layout.as_pixels;
}

ssize_t
ply_image_get_size(ply_image_t *image)
{
    assert(image != NULL);

    return image->size;
}

long ply_image_get_width(ply_image_t *image)
{
    assert(image != NULL);

    return image->width;
}

long ply_image_get_height(ply_image_t *image)
{
    assert(image != NULL);

    return image->height;
}

ply_image_t *
ply_image_resize(ply_image_t *image,
                 long width,
                 long height)
{
    ply_image_t *new_image;
    int x, y;
    int old_x, old_y, old_width, old_height;
    float scale_x, scale_y;

    new_image = ply_image_new(image->filename);

    new_image->width = width;
    new_image->height = height;

    if (width % 16 != 0)
    {
        width = (width + (16 - width % 16));
    }

    new_image->layout.address = malloc(height * width * 4);

    old_width = ply_image_get_width(image);
    old_height = ply_image_get_height(image);

    scale_x = ((double)old_width) / width;
    scale_y = ((double)old_height) / height;

    for (y = 0; y < height; y++)
    {
        old_y = y * scale_y;
        for (x = 0; x < width; x++)
        {
            old_x = x * scale_x;
            new_image->layout.as_pixels[x + y * width] = image->layout.as_pixels[old_x + old_y * old_width];
        }
    }
    return new_image;
}

ply_image_t *
ply_image_rotate(ply_image_t *image,
                 long center_x,
                 long center_y,
                 double theta_offset)
{
    ply_image_t *new_image;
    int x, y;
    int old_x, old_y;
    int width;
    int height;

    width = ply_image_get_width(image);
    height = ply_image_get_height(image);

    new_image = ply_image_new(image->filename);

    new_image->layout.address = malloc(height * width * 4);
    new_image->width = width;
    new_image->height = height;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            double d;
            double theta;

            d = sqrt((x - center_x) * (x - center_x) + (y - center_y) * (y - center_y));
            theta = atan2(y - center_y, x - center_x);

            theta -= theta_offset;
            old_x = center_x + d * cos(theta);
            old_y = center_y + d * sin(theta);

            if (old_x < 0 || old_x >= width || old_y < 0 || old_y >= height)
                new_image->layout.as_pixels[x + y * width] = 0x00000000;
            else
                new_image->layout.as_pixels[x + y * width] = image->layout.as_pixels[old_x + old_y * width];
        }
    }
    return new_image;
}