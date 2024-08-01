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
extern static_hash_map *map;
void construct_hash_map();
int add_key(int key, int value);
int get_value(int key);
int get_size();
int delete_key(int key);
void print_hash_map();
void free_memory_hash_map();
