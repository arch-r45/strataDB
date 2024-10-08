#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct {
    int key;
    int value;
}map_item;
typedef struct{
    map_item * hash_map;
    int current_occupation;
    int total_size;
} static_hash_map;
static_hash_map* construct_hash_map(int size){
    printf("Construction Called \n");
    static_hash_map *map = (static_hash_map*)malloc(sizeof(static_hash_map));
    printf("Address: %p\n", map);
    map_item mapping;
    mapping.key = -1;
    mapping.value = -1;
    map->hash_map = (map_item*)malloc(sizeof(map_item) * size);
    map->current_occupation = 0;
    map->total_size = size;
    for (int i = 0; i < map->total_size; i++){
        map->hash_map[i] = mapping; 
    }
    return map;
} 
int hash_function(int key,int total_size){
    return (total_size-1) % key;
}
int add_key(static_hash_map *map, int key, int value){
    if (map->current_occupation == map->total_size){
        return -1;
    }
    int hash_value = hash_function(key, map->total_size);
    printf("Hash Function %d \n", hash_value);
    printf("actual key %d \n", key);
    printf("Hash key %d \n", map->hash_map[hash_value].key);
    while (map->hash_map[hash_value].key != -1 & map->hash_map[hash_value].value != -1){
        printf("Hash Function %d \n", hash_value);
        if (map->hash_map[hash_value].key == key){
            map->hash_map[hash_value].value = value;
            return 0;
        }
        if (hash_value + 1 == map->total_size){
            hash_value = 0;
        }
        else{
            hash_value ++;
        }   
        }
    map_item mapping;
    mapping.key = key;
    mapping.value = value;

    map->hash_map[hash_value] = mapping;
    map->current_occupation ++;
    return 0;
}
int get_value(static_hash_map *map, int key){
    int hash_value = hash_function(key, map->total_size);
    int original_hash = hash_value;
    while (map->hash_map[hash_value].key != -1){
        if (hash_value == original_hash -1){
            break;
        }
        if (map->hash_map[hash_value].key == key && map->hash_map[hash_value].value != -1){
            return map->hash_map[hash_value].value;
        }
        if (map->hash_map[hash_value].key == key && map->hash_map[hash_value].value == -1){
            //ran into a tombstone meaning value does not exist
            break;
        }
        if (hash_value + 1 == map->total_size){
            hash_value = 0;
        }
        else{
            hash_value ++;
        }   
        }
    printf("No value Found for key %d \n", key);
    return -1;
}
int get_size(static_hash_map *map){
    return map->current_occupation;
}
int delete_key(static_hash_map *map, int key){
    int hash_value = hash_function(key, map->total_size);
    int original_hash = hash_value;
    while (map->hash_map[hash_value].key != -1){
        if (hash_value == original_hash -1){
            break;
        }
        if (map->hash_map[hash_value].key == key){
            map->current_occupation -= 1;
            map->hash_map[hash_value].value = -1;
            return 1;
        }
        if (hash_value + 1 == map->total_size){
            hash_value = 0;
        }
        else{
            hash_value ++;
        }
    }
    printf("Deleted key: %d not found \n", key);
    return -1;
}
void print_hash_map(static_hash_map *map){
    printf("current occupation %d\n", map->current_occupation);
    printf("{");
    int printed_keys = 0;
    for (int i=0; i < map->total_size; i++){
        if (map->hash_map[i].key != -1){
            printed_keys ++;
            printf("%d:%d", map->hash_map[i].key, map->hash_map[i].value);
            if (printed_keys != map->current_occupation){
                printf(",");
            }
        }
    }
    printf("}\n");
}
void free_memory_hash_map(static_hash_map *map) {
    if (map != NULL) {
        if (map->hash_map != NULL) {
            printf("Not null \n");
            free(map->hash_map);
            map->hash_map = NULL;
        }
        free(map);
        map = NULL;
    }
}

