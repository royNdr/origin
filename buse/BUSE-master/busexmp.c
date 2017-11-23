/*
 * busexmp - example memory-based block device using BUSE
 * Copyright (C) 2013 Adam Cozzette
 *
 * This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#define _XOPEN_SOURCE 800
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "buse.h"

static void *data;
static int fd_s;
static int xmpl_debug = 1;

static int xmp_read(void *buf, u_int32_t len, u_int64_t offset, void *userdata)
{
  if (*(int *)userdata)
    fprintf(stderr, "R - %lu, %u\n", offset, len);
  memcpy(buf, (char *)data + offset, len);
  return 0;
}

static int xmp_write(const void *buf, u_int32_t len, u_int64_t offset, void *userdata)
{
  if (*(int *)userdata)
    fprintf(stderr, "W - %lu, %u\n", offset, len);
  memcpy((char *)data + offset, buf, len);

  write(fd_s, "T\n", sizeof("T\n"));
 
  if(fd_s)
  {
    int byte_writen = 0;
    fprintf(stderr, "writing to file %d", fd_s);
    //lseek(fd, offset, SEEK_SET);
    //byte_writen = write(fd, "Y\n", sizeof("Y\n"));
    fprintf(stderr, " %d bytes.\n", byte_writen);
  }

  return 0;
}

static void xmp_disc(void *userdata)
{
  (void)(userdata);
  fprintf(stderr, "Received a disconnect request.\n");
}

static int xmp_flush(void *userdata)
{
  (void)(userdata);
  fprintf(stderr, "Received a flush request.\n");
  return 0;
}

static int xmp_trim(u_int64_t from, u_int32_t len, void *userdata)
{
  (void)(userdata);
  fprintf(stderr, "T - %lu, %u\n", from, len);
  return 0;
}


static struct buse_operations aop = {
  .read = xmp_read,
  .write = xmp_write,
  .disc = xmp_disc,
  .flush = xmp_flush,
  .trim = xmp_trim,
  .size = 128 * 1024 * 1024,
};

int main(int argc, char *argv[])
{
  int err = 0;

  if (argc != 2)
  {
    fprintf(stderr, 
        "Usage:\n"
        "  %s /dev/nbd0\n"
        "Don't forget to load nbd kernel module (`modprobe nbd`) and\n"
        "run example from root.\n", argv[0]);
    return 1;
  }

  data = malloc(aop.size);

  fd_s = open("buse_diskfile", O_CREAT|O_RDWR|O_TRUNC, S_IRWXU|S_IRWXG|S_IROTH);
  assert(fd_s != -1);
  err = ftruncate(fd_s, 100/*aop.size*/);
  lseek(fd_s, 0, SEEK_SET);
  write(fd_s, "R\n", sizeof("R\n"));

  assert(0 == err);

  close(fd_s);

  return buse_main(argv[1], &aop, (void *)&xmpl_debug);
}
