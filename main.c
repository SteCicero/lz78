/* main.c
Created by Stefano Cicero on 30/11/2016.
Copyright © 2016 Stefano Cicero. All rights reserved.
*/

#include "lz78.h"

int collision_number;

// Usage program help
void help(char* argv[])
{
    fprintf(stderr,
            "Usage: %s [Options]\n\n"
            "Options:\n"
            "-h          show this help\n"
            "-i input    sets input source\n"
            "-o output   sets output destination\n"
            "-d          sets decompress mode\n"
            "\n",
            argv[0]);
}

int main(int argc, char *argv[])
{
  char* fname_in = NULL;
  char* fname_out = NULL;
  int opt;
  bool decompress_mode = false;

  //parsing command line parameters
  while ((opt = getopt(argc, argv, "i:o:d")) != -1)
  {
    switch (opt)
    {
      case 'i':              //Input
        fname_in = optarg;
        break;

      case 'o':             //Output
        fname_out = optarg;
        break;

      case 'd':             //Decompress
        decompress_mode = true;
        break;

      case 'h':             //Compressor help
      default:
        help(argv);
        exit(EXIT_FAILURE);
    }
  }

  if(decompress_mode)
  {
    printf("Decompressing...\n");
    decompress(fname_in, fname_out);
    printf("File decompressed correctly. Done.\n");
  }
  else
  {
    printf("Compressing...\n");
    compress(fname_in, fname_out);
    printf("File compressed correctly. Done.\n");
  }
  return 0;
}
