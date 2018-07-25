/* lz78.c
Created by Stefano Cicero on 30/11/2016.
*/
#include "lz78.h"
#include <sys/stat.h>

#define ACCESS_READ (O_RDONLY | O_NONBLOCK)


void decompress(char* fname_in, char* fname_out)
{
  FILE* out;
  BITFILE* in;
  int fd_out = open(fname_out, O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0644);
  out = fdopen(fd_out, "w");
  in = bitfile_open(fname_in, 0);

  struct stat st;
  stat(fname_in, &st);
  int progress = 0;

  uint32_t dict_size;
  d_dictionary *dict;

  int current_node_index = 0;
  int previous_node_index = 0;
  uint32_t symbol = 0;
  uint8_t length = 9;
  int ret;
  int n_read = 0;
  unsigned int bit_read = DICT_HEADER_SIZE;

  //checking for errors in file opening
  if(out < 0)
  {
    printf("Error while trying to open output file, terminating...\n");
    exit(-1);
  }
  if(in == NULL)
  {
    printf("Error while trying to open input file, terminating...\n");
    exit(-1);
  }

  //reading dictionary maximum size from the input file
  ret = bitfile_read(in, (unsigned char*)&dict_size, DICT_HEADER_SIZE);

  if(ret < DICT_HEADER_SIZE)
  {
    printf("Error while trying to read the size of dictionary from input file. Terminating.\n");
    exit(-1);
  }

  //dictionary allocation and initialization
  dict = d_dict_alloc(dict_size);
  d_dict_init(dict);

  printf("Progress: 0%%\n");

  while(true)
  {
    symbol = 0;
    if((d_dict_get_size(dict)+1) >= (1 << length))
    {
      length++;
    }

    ret = bitfile_read(in, (unsigned char*)&symbol, length);

    n_read++;
    bit_read += ret;

    //progress status update
    if(progress < (uint32_t)(100*(bit_read/(st.st_size*8))))
    {
      progress = (100*(bit_read/(st.st_size*8)));
      fputs("\033[A\033[2K",stdout);
      rewind(stdout);
      printf("Progress: %i%% (%i/%lli) bytes\n",progress, (uint32_t)bit_read/8, st.st_size);
    }
    if(ret < length) break;

    previous_node_index = current_node_index;
    current_node_index = symbol;

    //Found reset symbol, resetting dictionary
    if(current_node_index == 0)
    {
      length = 9;
      d_dict_init(dict);
      continue;
    }
    //Found a symbol which isn't into the tree yet
    if(current_node_index == d_dict_get_size(dict))
    {
      d_dict_climb_update(dict, previous_node_index);
      string_bw(dict, current_node_index, out);
    }
    else
    {
      symbol = string_bw(dict, current_node_index, out);
      if(previous_node_index != 0)
      {
        d_dict_update(dict, previous_node_index, symbol);
      }
    }
  }
  bitfile_close(in);
  fclose(out);
  close(fd_out);
  if(bit_read < st.st_size*8)
  {
    printf("Error while trying to read the input file. Terminating.\n");
    exit(-1);
  }
}

void compress(char* fname_in, char* fname_out)
{
  FILE* in;
  int fd_in;
  BITFILE* out;
  int c_in;

  fd_in = open(fname_in, ACCESS_READ);
  in = fdopen(fd_in, "r");
  out = bitfile_open(fname_out, 1);

  uint32_t dict_size = DICT_SIZE;
  struct stat st;
  int progress = 0;

  c_dictionary *dict;

  int search_result;

  int current_node_index = 0;
  int n_write = 0;
  unsigned long int n_read = 0;
  bool sym_found = true;
  uint8_t length = 9;

  //checking for errors in file opening
  if(fd_in < 0)
  {
    printf("Error while trying to open input file, terminating...\n");
    exit(-1);
  }
  if(out == NULL)
  {
    printf("Error while trying to open output file, terminating...\n");
    exit(-1);
  }

  //writing dictionary maximum size to the output file
  bitfile_write(out, (unsigned char *)&dict_size, DICT_HEADER_SIZE);

  //retrieving input file size
  stat(fname_in, &st);

  //dictionary allocation and initialization
  dict = c_dict_alloc();
  c_dict_init(dict);

  printf("Progress: 0%%\n");

  while(true)
  {
    if(c_dict_get_size(dict) >= (1 << (length)))
    {
      length++;
    }

    int ffp = -1;           //first free position found by the search function
    if(sym_found)
    {
      c_in = fgetc(in);
      n_read++;

      //progress status update
      if(progress < (100*n_read/st.st_size))
      {
        progress = (100*n_read/st.st_size);
        fputs("\033[A\033[2K",stdout);
        rewind(stdout);
        printf("Progress: %i%% (%lu/%lli) bytes\n",progress, n_read, st.st_size);
      }
    }
    if(c_in == EOF)   //end of file reached
    {
      //writes the last symbol (if exist) before terminating
      if(sym_found)
      {
        int node_id = c_dict_get_node_id(dict, current_node_index);
        bitfile_write(out, (unsigned char *)&node_id, length);
        n_write++;
      }
      break;
    }
    search_result = c_dict_search(dict, current_node_index, c_in, &ffp);
    if(search_result != -1)
    {
      current_node_index = search_result;
      sym_found = true;
    }
    else
    {
      //inserting a new node
      c_dict_insert(dict, ffp, current_node_index, c_in);

      //writing symbol to the output file
      int node_id = c_dict_get_node_id(dict, current_node_index);
      bitfile_write(out, (unsigned char *)&node_id, length);
      n_write++;

      //current_node_index = dict.root_index;
      current_node_index = 0;
      sym_found = false;

      //dictionary full, perform a reset
      if(c_dict_get_size(dict) >= (DICT_SIZE*DICT_MAX_USAGE_RATIO))
      {
        int null_symbol = 0;
        bitfile_write(out, (unsigned char *)&null_symbol, length);
        length = 9;
        c_dict_init(dict);
      }
    }
  }
  /*printf("Caratteri letti: %i\n", n_read);
  printf("Simboli scritti: %i\n", n_write);
  printf("Numero collisioni: %i\n", collision_number);*/
  close(fd_in);
  bitfile_close(out);
}
