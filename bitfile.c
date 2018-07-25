/* bitfile.c
Created by Stefano Cicero on 30/11/2016.
*/
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "bitfile.h"

//buffer sizes in bytes and bits
#define BITBUF_SIZE 4096
#define BITBUF_SIZE_IN_BITS (BITBUF_SIZE * 8)

extern int errno ;

struct __bitbuf
{
  int first;                    //first valid bit in buffer
  int empty;                    //first free bit in buffer
  int mode;                     //0 = READ; 1 = WRITE
  int file_descriptor;          //OS file descriptor for sys call
  unsigned char buffer[BITBUF_SIZE];
};

unsigned READ_BIT(const unsigned char *base, unsigned i)
{
  unsigned char mask;
  base += i >> 3;
  mask = 1 << (i & 7);
  return *base & mask;
}

void WRITE_BIT(unsigned char *base, unsigned i, int val)
{
  unsigned char mask, d;
  mask = val ? 1 << (i&7) : 0;
  d = base[i >> 3] & ~(1 << (i & 7));
  base[i >> 3] = d | mask;
}

void bit_copy(const unsigned char *src, int src_offset, unsigned char *dst, int dst_offset, int len)
{
  int i;
  for(i = 0; i < len; i++)
  {
    WRITE_BIT(dst, dst_offset++, READ_BIT(src, src_offset++));
  }
}

int min(int x1, int x2)
{
  return x1<=x2?x1:x2;
}


BITFILE* bitfile_open(char* filename, int mode)
{
  int fd = -1;
  BITFILE *bfp = NULL;

  //checks on filename and mode
  if(filename == NULL || filename[0] == '\0') return NULL;
  if(mode != 0 && mode != 1) return NULL;

  //file opening
  if(mode == 0)
  {
    fd = open(filename, O_RDONLY);
  }
  else
  {
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  }
  if(fd < 0) return NULL;

  //descriptor allocation: calloc allocates and initializes to zero
  bfp = (BITFILE*)calloc(1, sizeof(BITFILE));
  if(bfp == NULL)
  {
    close(fd);
  }
  else
  {
    bfp->mode = mode;
    bfp->file_descriptor = fd;
  }
  //returning pointer
  return bfp;
}

int bitfile_read(BITFILE* bfp, unsigned char* dst, int dst_len)
{
  int dst_offset = 0;
  int len;
  for(;dst_len > 0;)
  {
    if(bfp->first == BITBUF_SIZE_IN_BITS || bfp->first == 0)
    {
      int l = read(bfp->file_descriptor, bfp->buffer, BITBUF_SIZE);
      if(l<=0) break;
      bfp->first = 0;
      bfp->empty = l*8;
    }
    len = min(bfp->empty - bfp->first, dst_len);
    if(len == 0) break;
    bit_copy(bfp->buffer, bfp->first, dst, dst_offset, len);
    bfp->first += len;
    dst_offset += len;
    dst_len -= len;
  }
  return dst_offset;
}

int bitfile_write(BITFILE* bfp, const unsigned char* src, int src_len)
{
  int src_offset = 0;
  for(; src_len > 0;)
  {
    //we can move the minimum number of bits between src_len
    //and the free space of the BITFILE buffer
    int len = min(src_len, BITBUF_SIZE_IN_BITS - bfp->empty);
    bit_copy(src, src_offset, bfp->buffer, bfp->empty, len);
    src_offset += len;
    bfp->empty += len;
    src_len -= len;

    //is the buffer ready to be written to disk?
    if(bfp->empty == BITBUF_SIZE_IN_BITS)
    {
      write(bfp->file_descriptor, bfp->buffer, BITBUF_SIZE);
      bfp->empty = 0;
    }
  }
  return src_offset;
}

int bitfile_close(BITFILE* bfp)
{
  int err = 0;
  if(bfp == NULL) return EINVAL;

  if(bfp->mode == 1 && bfp->empty > 0)
  {
    //it is necessary to write the last byte to the disk
    //compute the size to the next multiple of a byte; bfp->first is 0
    int n = (bfp->empty + 7)/8;
    int l = write(bfp->file_descriptor, bfp->buffer, n);
    if(n != l)
    {
      err = EINVAL;
    }
  }
  close(bfp->file_descriptor);
  memset(bfp, 0, sizeof(BITFILE));
  free(bfp);
  return err;
}
