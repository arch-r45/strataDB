
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "dynamic_hash_map_string_array.h"

typedef struct {
    int * key;
    static_hash_map_array *value;
}master_map_item_array;
typedef struct{
    master_map_item_array * hash_map;
    int current_occupation;
    int total_size;
} master_hash_map_array;
master_hash_map_array* master_construct_hash_map_array();
int master_hash_function_array(int key,int total_size);
int master_add_key_array(master_hash_map_array *map, int *key, static_hash_map_array *value);
static_hash_map_array * master_get_value_array(master_hash_map_array *map, int * key);
int master_get_size_array(master_hash_map_array *map);
int master_delete_key_array(master_hash_map_array *map, int key);
void free_memory_hash_map_array(master_hash_map_array* map);