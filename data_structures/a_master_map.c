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

master_hash_map_array* master_construct_hash_map_array(){
    //printf("Construction Called \n");
    master_hash_map_array*map = (master_hash_map_array*)malloc(sizeof(master_hash_map_array));
    //printf("Address: %p\n", map);
    master_map_item_array mapping;
    mapping.key = NULL;
    mapping.value = NULL;
    int size = 2;
    map->hash_map = (master_map_item_array*)malloc(sizeof(master_map_item_array) * size);
    map->current_occupation = 0;
    map->total_size = size;
    for (int i = 0; i < map->total_size; i++){
        map->hash_map[i] = mapping; 
        //printf("mapping %d : %s\n", i, map->hash_map[i].key);
    }
    //printf("Pointer to map %p \n", map);
    return map;
}
int master_hash_function_array(int key,int total_size){
    //printf("total size: %d\n", total_size);
    return key% total_size;
}
int master_add_key_resize_array(master_hash_map_array* new_map, int key, static_hash_map_array* value){
    //printf("Pointer to map add_key %p \n", new_map);
    //printf("total size : %d\n", new_map->total_size);
    int hash_value = master_hash_function_array(key, new_map->total_size);
    //printf("Hash Function %d \n", hash_value);
    //printf("Hash key %s \n", new_map->hash_map[hash_value].key);
    while (new_map->hash_map[hash_value].key != NULL && new_map->hash_map[hash_value].value != NULL){
        /*
        printf("KEY %s\n",new_map->hash_map[hash_value].key);
        printf("resize \n");
        printf("Current occupation %d \n",new_map->current_occupation);
        printf("Total Capacity %d \n",new_map->total_size);
        printf("Hash Function %d \n", hash_value);
        */
        if (new_map->hash_map[hash_value].key == key){
            new_map->hash_map[hash_value].value = value;
            return 0;
        }
        if (hash_value + 1 == new_map->total_size){
            hash_value = 0;
        }
        else{
            hash_value ++;
        }   
        }
    master_map_item_array mapping;
    mapping.key = key;
    mapping.value = value;

    new_map->hash_map[hash_value] = mapping;
    new_map->current_occupation ++;
    return 0;
}

master_hash_map_array * master_resize_array(master_hash_map_array *map){
    //printf("Resize called \n");
    //printf("Pointer to map inside resize %p \n", map);
    int new_size = map->total_size * 2;
    master_hash_map_array *new_map = (master_hash_map_array*)malloc(sizeof(master_hash_map_array));
    new_map->hash_map = (master_map_item_array*)malloc(sizeof(master_map_item_array) * new_size);
    new_map->current_occupation = 0;
    new_map->total_size = new_size;
    master_map_item_array mapping;
    mapping.key = NULL;
    mapping.value = NULL;
    for (int i = 0; i < new_map->total_size; i++){
        new_map->hash_map[i] = mapping; 
    }
    //printf("Map current ocu %d\n", map->current_occupation);
    for (int i=0; i < map->total_size; i++){
        //printf("i%d: %s\n", i, map->hash_map[i].key);
        if (map->hash_map[i].key != NULL && map->hash_map[i].value != NULL){
            //printf("resize key %s\n", map->hash_map[i].key);
            master_add_key_resize_array(new_map, map->hash_map[i].key, map->hash_map[i].value);
        }
    }
    map->hash_map = new_map->hash_map;
    map->current_occupation = new_map->current_occupation;
    map->total_size = new_map->total_size;
    return map;
}


int master_add_key_array(master_hash_map_array *map, int *key, static_hash_map_array *value){
    //printf("Pointer to map add_key %p \n", map);
    if (map->current_occupation == (map->total_size >> 1)){
        //printf("Current occupation %d \n",map->current_occupation);
        map = resize_array(map);
        //printf("Pointer to map post resize %p \n", map);
    }
    int hash_value = master_hash_function_array(key, map->total_size);
    /*
    printf("Hash Function %d \n", hash_value);
    printf("actual key %s \n", key);
    printf("Hash key %s \n", map->hash_map[hash_value].key);
    */
    while (map->hash_map[hash_value].key != NULL && map->hash_map[hash_value].value != NULL){
        /*
        printf("KEY %s\n",map->hash_map[hash_value].key);
        printf("Map total size %d \n", map-> total_size);
        printf("Current occupation %d \n",map->current_occupation);
        printf("Total Capacity %d \n",map->total_size);
        printf("Hash Function %d \n", hash_value);
        */
        if (map->hash_map[hash_value].key == key){
            //printf("Same Key %s\n", key);
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
    master_map_item_array mapping;
    mapping.key = key;
    mapping.value = value;

    map->hash_map[hash_value] = mapping;
    map->current_occupation ++;
    return 0;
}

static_hash_map_array * master_get_value_array(master_hash_map_array *map, int * key){
    //printf("Key: %s\n", key);
    int hash_value = master_hash_function_array(key, map->total_size);
    //printf("Hash Value %d \n", hash_value);
    int original_hash = hash_value;
    while (map->hash_map[hash_value].key != NULL){
        if (hash_value == original_hash -1){
            break;
        }
        if (map->hash_map[hash_value].key == key && map->hash_map[hash_value].value != NULL){
            return map->hash_map[hash_value].value;
        }
        if (map->hash_map[hash_value].key == key && map->hash_map[hash_value].value == NULL){
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
    return NULL;
}

int master_get_size_array(master_hash_map_array *map){
    return map->current_occupation;
}

int master_delete_key_array(master_hash_map_array *map, int key){
    int hash_value = master_hash_function_array(key, map->total_size);
    int original_hash = hash_value;
    while (map->hash_map[hash_value].key != NULL){
        if (hash_value == original_hash -1){
            break;
        }
        if (map->hash_map[hash_value].key == key){
            map->hash_map[hash_value].value = NULL;
            return 1;
        }
        if (hash_value + 1 == map->total_size){
            hash_value = 0;
        }
        else{
            hash_value ++;
        }
    }
    //printf("Deleted key: %s not found \n", key);
    return -1;
}


void free_memory_hash_map_array(master_hash_map_array* map) {
    if (map != NULL) {
        if (map->hash_map != NULL) {
            free(map->hash_map);
            map->hash_map = NULL;
        }
        free(map);
        map = NULL;
    }
}

