/* bitfile.h
Created by Stefano Cicero on 30/11/2016.
*/

#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>

struct __bitbuf;

typedef struct __bitbuf BITFILE;

//open a file
BITFILE* bitfile_open(char* filename, int mode);

//read a certain number of bits
int bitfile_read(BITFILE* bfp, unsigned char* dst, int src_len);

//write a certain number of bits
int bitfile_write(BITFILE* bfp, const unsigned char* src, int dst_len);

//close a file
int bitfile_close(BITFILE* bfp);
