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
//extern char null_value[];
static_hash_map* construct_hash_map();
//int hash_function(int key,int total_size);
int add_key(static_hash_map* map, char *key, char *value);
char * get_value(static_hash_map* map, char* key);
int get_size(static_hash_map* map);
int delete_key(static_hash_map* map, char * key);
void print_hash_map(static_hash_map* map);
void free_memory_hash_map(static_hash_map* map);
