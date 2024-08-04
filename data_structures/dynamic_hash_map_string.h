#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
    char * key;
    char * value;
}map_item;
typedef struct{
    map_item * hash_map;
    int current_occupation;
    int total_size;
} static_hash_map;
extern static_hash_map *map;
extern char null_value[];
void construct_hash_map();
int hash_function(int key,int total_size);
static_hash_map* resize(static_hash_map *map);
int add_key(char *key, char *value);
char * get_value(char* key);
int get_size();
int delete_key(char * key);
void print_hash_map();
void free_memory_hash_map();
