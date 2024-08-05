#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
    char * key;
    int *value;
}map_item;
typedef struct{
    map_item * hash_map;
    int current_occupation;
    int total_size;
} static_hash_map;
extern int null_value[];
extern int nan_value;
static_hash_map* construct_hash_map_array();
int hash_function(int key,int total_size);
static_hash_map* resize_array(static_hash_map *map);
int add_key_array(static_hash_map* map, char *key, int *value);
int* get_value_array(static_hash_map* map, char* key);
int get_size_array(static_hash_map* map);
int delete_key_array(static_hash_map* map, char* key);
void print_hash_map_array(static_hash_map* map);
void free_memory_hash_map_array(static_hash_map* map);