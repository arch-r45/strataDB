#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
typedef struct link_node{
    char * value;
    struct link_node * next;
    struct link_node * prev;
    int pin;
}link_node;
typedef struct {
    char * key;
    link_node * value;
}lru_map_item;
typedef struct{
    lru_map_item * hash_map;
    int current_occupation;
    int total_size;
} lru_static_hash_map;
extern char lru_null_value;
lru_static_hash_map* lru_construct_hash_map();
int lru_hash_function(int key,int total_size);
lru_static_hash_map* lru_resize(lru_static_hash_map *map);
int lru_add_key(lru_static_hash_map* map, char *key, link_node *value);
link_node * lru_get_value(lru_static_hash_map* map, char* key);
int lru_get_size(lru_static_hash_map* map);
int lru_delete_key(lru_static_hash_map* map, char* key);
void lru_print_hash_map(lru_static_hash_map* map);
void lru_free_memory_hash_map(lru_static_hash_map* map);