#ifndef _HAVE_PLYMOUTH_H
#define _HAVE_PLYMOUTH_H
#define _GNU_SOURCE 1
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

#include "ply-bar.h"
#include "ply-color.h"

#define MSG_FILE_PATH ""
#define MSG_FILE_MAX_LEN 32
#define MSG_FILE_PREFIX ""
#define MSG ""
#define SPLIT_LINE_POS(buffer)

typedef uint8_t uint8;

#define PLYMOUTH_FIFO "plymouth_fifo"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
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

#ifdef __GNUC__
#define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_##x
#endif

#define DEBUG 1

#if DEBUG
#define DBG(x, a...)                                                  \
    {                                                                 \
        printf(__FILE__ ":%d,%s() " x "\n", __LINE__, __func__, ##a); \
    }
#else
#define DBG(x, a...) \
    do               \
    {                \
    } while (0)
#endif

#endif