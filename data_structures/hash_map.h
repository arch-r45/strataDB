#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
typedef struct {
    int key;
    int value;
}map_item;
typedef struct{
    map_item * hash_map;
    int current_occupation;
    int total_size;
} static_hash_map;

static_hash_map* construct_hash_map(int size);
int hash_function(int key,int total_size);
int add_key(static_hash_map *map, int key, int value);
int get_value(static_hash_map *map, int key);
int get_size(static_hash_map *map);
int delete_key(static_hash_map *map, int key);
void print_hash_map(static_hash_map *map);
void free_memory_hash_map(static_hash_map *map);

