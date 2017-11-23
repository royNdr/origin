#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

int main()
{
    puts("Starting....");
    
    int fd = open("test_file", O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);

    puts("truncating....");
    int err = ftruncate(fd, 1024);
    printf("trancate return value: %d\n", err);

    puts("writing....");
    write(fd, "test\n", sizeof("test\n"));

    close(fd);

    puts("end....");

    return 0;
}

