#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
    char * key;
    int *value;
}map_item_array;
typedef struct{
    map_item_array * hash_map;
    int current_occupation;
    int total_size;
} static_hash_map_array;
extern char null_value_array[];
extern int nan_value;
static_hash_map_array* construct_hash_map_array();
int hash_function_array(int key,int total_size);
static_hash_map_array* resize_array(static_hash_map_array *map);
int add_key_array(static_hash_map_array* map, char *key, int *value);
int* get_value_array(static_hash_map_array* map, char* key);
int get_size_array(static_hash_map_array* map);
int delete_key_array(static_hash_map_array* map, char* key);
void print_hash_map_array(static_hash_map_array* map);
void free_memory_hash_map_array(static_hash_map_array* map);