#include "plymouth-lite.h"

int main(int argc, char **argv)
{
    char *tmpdir;
    int pipe_fd;

    tmpdir = getenv("TMPDIR");

    if (!tmpdir)
        tmpdir = "/tmp";

    if (argc != 2)
    {
        fprintf(stderr, "Wrong number of arguments\n");
        exit(-1);
    }

    chdir(tmpdir);

    if ((pipe_fd = open(PLYMOUTH_FIFO, O_WRONLY | O_NONBLOCK)) == -1)
    {
        /* Silently error out instead of covering the boot process in 
         errors when psplash has exitted due to a VC switch */

        perror("Error unable to open fifo");
        exit(-1);
    }

    write(pipe_fd, argv[1], strlen(argv[1]) + 1);

    return 0;
}