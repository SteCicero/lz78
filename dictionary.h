/* dictionary.h
Created by Stefano Cicero on 30/11/2016.
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define DICT_SIZE 50000             //dictionary size in #nodes
#define DICT_MAX_USAGE_RATIO 0.8    //max usage ratio of the array to reduce conflicts when hashing
#define DICT_HEADER_SIZE 20         //#bits reserved for DICT_SIZE

extern int collision_number;

//structure for dictionary compressor
struct c_dictionary;
typedef struct c_dictionary c_dictionary;

//structure for dictionary decompressor
struct d_dictionary;
typedef struct d_dictionary d_dictionary;

//allocates data structures for the compressor's dictionary
c_dictionary* c_dict_alloc();

//initializes compressor's dictionary
void c_dict_init(c_dictionary* dict);

//returns current size of compressor's dictionary
int c_dict_get_size(c_dictionary* dict);

//returns node id of a given index
int c_dict_get_node_id(c_dictionary* dict, int node_index);

//perform a search in the dictionary returning the index of the node found, -1
//otherwhise
int c_dict_search(c_dictionary* dict, int parent_index, uint8_t symbol, int* ffp);

//inserts a new node into the compressor's dictionary
void c_dict_insert(c_dictionary* dict, int ffp, int parent_index, char c_in);

//allocates data structures for the decompressor's dictionary
d_dictionary* d_dict_alloc(uint32_t dict_size);

//initializes decompressor's dictionary
void d_dict_init(d_dictionary* dict);

//returns current size of decompressor's dictionary
int d_dict_get_size(d_dictionary* dict);

//updates decompressor's dictionary if a symbol isn't in the dictionary yet
void d_dict_climb_update(d_dictionary* dict, int node_index);

//updates decompressor's dictionary
void d_dict_update(d_dictionary* dict, int node_index, char character);

//String build and write function: starting from an index climb the trie,
//build the string associated with the path and write the latter to the file.
//Returns the symbol of the node closest to the root
uint32_t string_bw(d_dictionary* dict, int index, FILE* out);
