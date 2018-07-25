/* dictionary.c
Created by Stefano Cicero on 30/11/2016.
*/
#include "dictionary.h"

int collision_number = 0;

struct c_node
{
  int parent_index;   //index of the parent node
  int id;             //identifier (symbol) associated to the node
  uint8_t character;  //character associated with the node
  bool used;          //indicates if the node is currently used or not
};

struct c_dictionary
{
  uint32_t counter;     //number of nodes used
  int root_index;       //index of root trie
  struct c_node *nodes; //array of dictionary nodes
};

struct d_node
{
  int parent_index;     //index of the parent node
  uint8_t character;    //character associated with the node
  bool used;            //indicates if the node is currently used or not
};

struct d_dictionary
{
  uint32_t counter;     //number of nodes used
  struct d_node* nodes; //array of dictionary nodes
};

//Bernstein hash function
unsigned long hash(unsigned char *str, int length)
{
    unsigned long h = 0;
    int i;
    for(i=0; i < length; i++)
    {
      h = 33 * (h + str[i]);
    }
    return h%DICT_SIZE;
}

//allocates data structures for the compressor's dictionary
c_dictionary* c_dict_alloc()
{
  c_dictionary *dict = (c_dictionary*)calloc(1, sizeof(c_dictionary));;
  dict->nodes = malloc(DICT_SIZE * (sizeof(struct c_node)));
  dict->counter = 0;
  return dict;
}

void c_dict_init(c_dictionary* dict)
{
  //setting all nodes as free
  for(int i=0; i<DICT_SIZE; i++)
  {
    dict->nodes[i].used = false;
  }

  //initializing root node
  dict->root_index = 0;
  dict->nodes[dict->root_index].parent_index = -1;
  dict->nodes[dict->root_index].id = 0;
  dict->nodes[dict->root_index].character = 0;
  dict->nodes[dict->root_index].used = true;

  //initializing all possible 256 characters
  for(int i=0; i<256; i++)
  {
    char str[2];
    unsigned long index;
    str[0] = '0';
    char tmp = i;
    str[1] = tmp;

    index = hash((unsigned char *)str, 2);
    dict->nodes[index].parent_index = dict->root_index;
    dict->nodes[index].id = i+1;
    dict->nodes[index].character = i;
    dict->nodes[index].used = true;
  }
  dict->counter = 257;
}

int c_dict_get_size(c_dictionary* dict)
{
    return dict->counter;
}

int c_dict_get_node_id(c_dictionary* dict, int node_index)
{
  return dict->nodes[node_index].id;
}

int c_dict_search(c_dictionary* dict, int parent_index, uint8_t symbol, int* ffp)
{
  int parent_id = dict->nodes[parent_index].id;
  int index;

  //the first level of the tree is reached throught a different hash key
  if(parent_index == 0)
  {
    char str[2];
    str[0] = '0';
    str[1] = symbol;
    index = hash((unsigned char *)str, 2);
  }
  else
  {
    unsigned char tmp[5];
    memcpy(tmp, &symbol, 1);
    memcpy(tmp+1, &parent_id, 4);
    index = hash((unsigned char*)&tmp, 5);
  }

  while(true)
  {
    if(!dict->nodes[index].used)
    {
      *ffp = index;
      return -1;
    }
    if(dict->nodes[index].character == symbol && dict->nodes[dict->nodes[index].parent_index].id  == parent_id)
    {
      return index;
    }
    collision_number++;
    index = (index + 1) % DICT_SIZE;
  }
  return -1;
}

void c_dict_insert(c_dictionary* dict, int ffp, int parent_index, char c_in)
{
  dict->nodes[ffp].used = true;
  dict->nodes[ffp].id = dict->counter;
  dict->nodes[ffp].parent_index = parent_index;
  dict->counter++;
  dict->nodes[ffp].character = c_in;
}

d_dictionary* d_dict_alloc(uint32_t dict_size)
{
  d_dictionary *dict = (d_dictionary*)calloc(1, sizeof(d_dictionary));;
  dict->nodes = malloc(dict_size * (sizeof(struct d_node)));
  dict->counter = 0;
  return dict;
}

void d_dict_init(d_dictionary* dict)
{
  for(int i=0; i<DICT_SIZE; i++)
  {
    dict->nodes[i].used = false;
  }

  dict->nodes[0].parent_index = -1;
  dict->nodes[0].character = 0;
  dict->nodes[0].used = true;

  for(int i=1; i<257; i++)
  {
    dict->nodes[i].parent_index = 0;
    dict->nodes[i].character = i-1;
    dict->nodes[i].used = true;
  }
  dict->counter = 257;
}

int d_dict_get_size(d_dictionary* dict)
{
  return dict->counter;
}

void d_dict_climb_update(d_dictionary* dict, int node_index)
{
  int tmp = node_index;

  while (dict->nodes[tmp].parent_index != 0)
  {
    tmp = dict->nodes[tmp].parent_index;
  }

  dict->nodes[dict->counter].parent_index = node_index;
  dict->nodes[dict->counter].character = dict->nodes[tmp].character;
  dict->nodes[dict->counter].used = true;
  dict->counter++;
}

void d_dict_update(d_dictionary* dict, int node_index, char character)
{
  dict->nodes[dict->counter].parent_index = node_index;
  dict->nodes[dict->counter].character = character;
  dict->nodes[dict->counter].used = true;
  dict->counter++;
}

uint32_t string_bw(d_dictionary* dict, int index, FILE* out)
{
  if(dict->nodes[index].parent_index == 0)
  {
    fwrite((unsigned char*)&dict->nodes[index].character, 1, 1, out);
    return dict->nodes[index].character;
  }
  else
  {
    int symbol = string_bw(dict, dict->nodes[index].parent_index, out);
    fwrite((unsigned char*)&dict->nodes[index].character, 1, 1, out);
    return symbol;
  }
}
