#ifndef _HAVE_PLYMOUTH_H
#define _HAVE_PLYMOUTH_H

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <math.h>
#include <png.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <values.h>

#define MSG_FILE_PATH "/tmp/essai"
#define MSG_FILE_MAX_LEN 32
#define MSG_FILE_PREFIX "KOS:"
#define MSG ""
#define SPLIT_LINE_POS(fb)

typedef uint8_t uint8;

#define PLYMOUTH_FIFO "plymouth_fifo"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef _HAVE_PSPLASH_COLORS_H
#define _HAVE_PSPLASH_COLORS_H

/* This is the overall background color */
#define PLYMOUTH_BACKGROUND_COLOR 0xec, 0xec, 0xe1

/* This is the color of any text output */
#define PLYMOUTH_TEXT_COLOR 0x6d, 0x6d, 0x70

/* This is the color of the progress bar indicator */
#define PLYMOUTH_BAR_COLOR 0x6d, 0x6d, 0x70

/* This is the color of the progress bar background */
#define PLYMOUTH_BAR_BACKGROUND_COLOR 0xec, 0xec, 0xe1

#endif

#ifndef FRAMES_PER_SECOND
#define FRAMES_PER_SECOND 50
#endif

typedef struct PlymouthFont
{
    char *name;     /* Font name. */
    int height;     /* Height in pixels. */
    int index_mask; /* ((1 << N) - 1). */
    int *offset;    /* (1 << N) offsets into index. */
    int *index;
    u_int32_t *content;
} PlymouthFont;

#endif