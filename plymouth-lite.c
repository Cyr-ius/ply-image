#include "ply-image.h"
#include "ply-config.h"
#include "ply-frame-buffer.h"
#include "ply-console.h"
#include "radeon-font.h"

static int console_fd;

static bool
hide_cursor(void)
{
  static const char invisible_cursor[] = "\033[?25l\033[?1c";

  if (write(STDOUT_FILENO, invisible_cursor,
            sizeof(invisible_cursor) - 1) != sizeof(invisible_cursor) - 1)
    return false;

  return true;
}

static void
animate_at_time(ply_frame_buffer_t *buffer,
                ply_image_t *image)
{
  ply_frame_buffer_area_t area;
  uint32_t *data;
  long width, height;

  data = ply_image_get_data(image);
  width = ply_image_get_width(image);
  height = ply_image_get_height(image);

  ply_frame_buffer_get_size(buffer, &area);
  area.x = (area.width / 2) - (width / 2);
  area.y = (area.height / 2) - (height / 2);
  area.width = width;
  area.height = height;

  ply_frame_buffer_pause_updates(buffer);
  ply_frame_buffer_fill_with_argb32_data(buffer, &area, 0, 0, data);
  ply_frame_buffer_unpause_updates(buffer);
}

static int
parse_command(ply_frame_buffer_t *buffer, char *string)
{
  char *command;

  if (strcmp(string, "QUIT") == 0)
    return 1;

  command = strtok(string, " ");

  if (!strcmp(command, "PROGRESS"))
  {
    // ply_draw_progress(buffer, atoi(strtok(NULL, "\0")));
  }
  else if (!strcmp(command, "MSG"))
  {
    // ply_draw_msg(buffer, strtok(NULL, "\0"));
  }
  else if (!strcmp(command, "QUIT"))
  {
    return 1;
  }

  return 0;
}

void plymouth_draw_msg(ply_frame_buffer_t *buffer, const char *msg)
{
  int w, h;

  ply_frame_buffer_text_size(&w, &h, &radeon_font, msg);

  /* Clear */

  ply_frame_buffer_draw_text(buffer,
                             (buffer->area.width - w) / 2,
                             SPLIT_LINE_POS(buffer) - h,
                             PLYMOUTH_TEXT_COLOR,
                             &radeon_font,
                             msg);

  bool ply_frame_buffer_flush(ply_frame_buffer_t * buffer);
}

void plymouth_main(ply_frame_buffer_t *buffer, int pipe_fd, int timeout)
{
  int err;
  ssize_t length = 0;
  fd_set descriptors;
  struct timeval tv;
  char *end;
  char command[2048];

  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  FD_ZERO(&descriptors);
  FD_SET(pipe_fd, &descriptors);

  end = command;

  while (1)
  {
    if (timeout != 0)
      err = select(pipe_fd + 1, &descriptors, NULL, NULL, &tv);
    else
      err = select(pipe_fd + 1, &descriptors, NULL, NULL, NULL);

    if (err <= 0)
    {
      /*
	  if (errno == EINTR)
	    continue;
	  */
      return;
    }

    length += read(pipe_fd, end, sizeof(command) - (end - command));

    if (length == 0)
    {
      /* Reopen to see if there's anything more for us */
      close(pipe_fd);
      pipe_fd = open(PLYMOUTH_FIFO, O_RDONLY | O_NONBLOCK);
      goto out;
    }

    if (command[length - 1] == '\0')
    {
      if (parse_command(buffer, command))
        return;
      length = 0;
    }
    else if (command[length - 1] == '\n')
    {
      command[length - 1] = '\0';
      if (parse_command(buffer, command))
        return;
      length = 0;
    }

  out:
    end = &command[length];

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    FD_ZERO(&descriptors);
    FD_SET(pipe_fd, &descriptors);
  }

  return;
}

int main(int argc,
         char **argv)
{
  ply_image_t *image;
  ply_frame_buffer_t *buffer;
  char *tmpdir;
  int i = 0, angle = 0, fbdev_id = 0, retcode = -1;
  int pipe_fd;
  bool disable_console_switch = FALSE;
  bool disable_message = FALSE;
  bool disable_progress_bar = FALSE;
  bool disable_logo = FALSE;
  bool disable_cursor = FALSE;
  FILE *fd_msg;
  char *str_msg;
  int exit_code;

  exit_code = 0;

  if (argc == 1)
  {
    return exit_code;
  }
  while (++i < argc)
  {
    if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--no-console-switch"))
    {
      disable_console_switch = TRUE;
      continue;
    }

    if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--hide-cursor"))
    {
      disable_cursor = TRUE;
      continue;
    }

    if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--angle"))
    {
      if (++i >= argc)
        goto fail;
      angle = atoi(argv[i]);
      continue;
    }

    if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--fbdev"))
    {
      if (++i >= argc)
        goto fail;
      fbdev_id = atoi(argv[i]);
      continue;
    }

    if (!strcmp(argv[i], "-m") || !strcmp(argv[i], "--no-message"))
    {
      disable_message = TRUE;
      continue;
    }

    if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--no-progress"))
    {
      disable_progress_bar = TRUE;
      continue;
    }

    if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--no-logo"))
    {
      disable_logo = TRUE;
      continue;
    }

    if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--angle"))
    {
      if (++i >= argc)
        goto fail;
      angle = atoi(argv[i]);
      continue;
    }

    if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--code"))
    {
      if (++i >= argc)
        goto fail;
      retcode = atoi(argv[i]);
      continue;
    }

    if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--image"))
    {
      if (++i >= argc)
        goto fail;
      char imgpath[255];
      strcpy(imgpath, argv[i]);
      image = ply_image_new(imgpath);
      continue;
    }

  fail:
    fprintf(stderr,
            "Usage: %s -c|--code <0|1|64|65|66|100> [-i|--image] path [-n|--no-console-switch][-m|--no-message][-p|--no-progress][-l|--no-logo][-a|--angle <0|90|180|270>]\n",
            argv[0]);
    exit(-1);
  }

  if (retcode >= 0)
  {
    switch (retcode)
    {
    case 0:
    case 64:
    case 65:
      image = ply_image_new("/usr/share/plymouth-lite/splash.png");
      break;
    case 66:
    case 1:
      image = ply_image_new("/usr/share/plymouth-lite/splash_crash.png");
      break;
    case 100:
      image = ply_image_new("/usr/share/plymouth-lite/splash_update.png");
      break;
    default:
      image = ply_image_new("/usr/share/plymouth-lite/splash_crash.png"); /* Unknown error */
    }
  }
  else if (!image)
  {
    image = ply_image_new("/usr/share/plymouth-lite/splash.png");
  }

  if (!ply_image_load(image))
  {
    exit_code = errno;
    perror("could not load image");
    return exit_code;
  }

  console_fd = open("/dev/tty0", O_RDWR);

  buffer = ply_frame_buffer_new(NULL);

  tmpdir = getenv("TMPDIR");

  if (!tmpdir)
    tmpdir = "/tmp";

  chdir(tmpdir);

  if (mkfifo(PLYMOUTH_FIFO, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP))
  {
    if (errno != EEXIST)
    {
      perror("mkfifo");
      exit(-1);
    }
  }

  pipe_fd = open(PLYMOUTH_FIFO, O_RDONLY | O_NONBLOCK);

  if (pipe_fd == -1)
  {
    perror("pipe open");
    exit(-2);
  }

  if (!ply_frame_buffer_open(buffer))
  {
    exit_code = errno;
    perror("could not open framebuffer");
    return exit_code;
  }

  image = ply_image_resize(image, buffer->area.width, buffer->area.height);

  animate_at_time(buffer, image);

  /* Disable curson on TTY */
  if (!disable_cursor)
  {
    hide_cursor();
  }

  /* Draw message from file or defined MSG */
  if (!disable_message)
  {
    fd_msg = fopen(MSG_FILE_PATH, "r");
    if (fd_msg == NULL)
    {
      plymouth_draw_msg(buffer, MSG);
    }
    else
    {
      str_msg = (char *)malloc(
          (MSG_FILE_MAX_LEN + strlen(MSG_FILE_PREFIX) + 1) * sizeof(char));
      if (str_msg != NULL && fgets(str_msg, MSG_FILE_MAX_LEN, fd_msg) != NULL)
      {
        if (strlen(MSG_FILE_PREFIX) > 0)
        {
          /* if MSG_FILE_PREFIX is set, prepend it to str_msg */
          memmove(str_msg + strlen(MSG_FILE_PREFIX) + 1, str_msg, strlen(str_msg));
          strcpy(str_msg, MSG_FILE_PREFIX);
          /* replace \0 after MSG_FILE_PREFIX with a space */
          str_msg[strlen(MSG_FILE_PREFIX)] = ' ';
        }
        plymouth_draw_msg(buffer, str_msg);
        free(str_msg);
      }
      else
      {
        plymouth_draw_msg(buffer, MSG_FILE_PREFIX);
      }
      fclose(fd_msg);
    }
  }

  // if (!disable_console_switch)
  // plymouth_console_switch();

#ifdef PLYMOUTH_STARTUP_MSG
  plymouth_draw_msg(buffer, PLYMOUTH_STARTUP_MSG);
#endif

  plymouth_main(buffer, pipe_fd, 0);

  ply_frame_buffer_close(buffer);
  ply_frame_buffer_free(buffer);

  ply_image_free(image);
  return exit_code;
}
