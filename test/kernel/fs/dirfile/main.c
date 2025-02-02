#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char buf[512];

int main(int argc, char** argv)
{
    int fd;

    fd = open("dirfile", 0);
    if (fd >= 0) {
        TestErr("create dirfile succeeded");
    }
    fd = open("dirfile", O_CREAT);
    if (chdir("dirfile") == 0) {
        TestErr("chdir dirfile succeeded");
    }
    if (unlink("dirfile") != 0) {
        TestErr("unlink dirfile succeeded");
    }

    fd = open(".", O_RDWR);
    if (fd >= 0) {
        TestErr("open . for writing succeeded");
    }
    fd = open(".", 0);
    if (write(fd, "x", 1) > 0) {
        TestErr("write . succeeded");
    }
    close(fd);

    return 0;
}