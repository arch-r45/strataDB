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
char lru_null_value[] = "3*0193!~#";
lru_static_hash_map* lru_construct_hash_map();
int lru_hash_function(int key,int total_size);
lru_static_hash_map* lru_resize(lru_static_hash_map *map);
int lru_add_key(lru_static_hash_map* map, char *key, link_node *value);
link_node * lru_get_value(lru_static_hash_map* map, char* key);
int lru_get_size(lru_static_hash_map* map);
int lru_delete_key(lru_static_hash_map* map, char* key);
void lru_print_hash_map(lru_static_hash_map* map);
void lru_free_memory_hash_map(lru_static_hash_map* map);

lru_static_hash_map* lru_construct_hash_map(){
    //printf("Construction Called \n");
    lru_static_hash_map*map = (lru_static_hash_map*)malloc(sizeof(lru_static_hash_map));
    //printf("Address: %p\n", map);
    lru_map_item mapping;
    mapping.key = lru_null_value;
    mapping.value = NULL;
    int size = 2;
    map->hash_map = (lru_map_item*)malloc(sizeof(lru_map_item) * size);
    map->current_occupation = 0;
    map->total_size = size;
    for (int i = 0; i < map->total_size; i++){
        map->hash_map[i] = mapping; 
        //printf("mapping %d : %s\n", i, map->hash_map[i].key);
    }
    //printf("Pointer to map %p \n", map);

    return map;
}
int lru_hash_function(int key,int total_size){
    //printf("total size: %d\n", total_size);
    return key% total_size;
}

int lru_add_key_resize(lru_static_hash_map* new_map, char* key, link_node* value){
    //printf("Pointer to map add_key %p \n", new_map);
    //printf("total size : %d\n", new_map->total_size);
    int new_key_length = strlen(key);
    int summation = 0;
    for (int i = 0; i < new_key_length; i++){
        summation += (int) key[i];
    }
    int hash_value = lru_hash_function(summation, new_map->total_size);
    //printf("Hash Function %d \n", hash_value);
    //printf("Hash key %s \n", new_map->hash_map[hash_value].key);
    while (new_map->hash_map[hash_value].key != lru_null_value && new_map->hash_map[hash_value].value != NULL){
        //printf("KEY %s\n",new_map->hash_map[hash_value].key);
        //printf("resize \n");
        //printf("Current occupation %d \n",new_map->current_occupation);
        //printf("Total Capacity %d \n",new_map->total_size);
        //printf("Hash Function %d \n", hash_value);
        if (strcmp(new_map->hash_map[hash_value].key, key) == 0){
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
    lru_map_item mapping;
    mapping.key = key;
    mapping.value = value;

    new_map->hash_map[hash_value] = mapping;
    new_map->current_occupation ++;
    return 0;
}

lru_static_hash_map * lru_resize(lru_static_hash_map *map){
    //printf("Resize called \n");
    //printf("Pointer to map inside resize %p \n", map);
    int new_size = map->total_size * 2;
    lru_static_hash_map *new_map = (lru_static_hash_map*)malloc(sizeof(lru_static_hash_map));
    new_map->hash_map = (lru_map_item*)malloc(sizeof(lru_map_item) * new_size);
    new_map->current_occupation = 0;
    new_map->total_size = new_size;
    lru_map_item mapping;
    mapping.key = lru_null_value;
    mapping.value = NULL;
    for (int i = 0; i < new_map->total_size; i++){
        new_map->hash_map[i] = mapping; 
    }
    //printf("Map current ocu %d\n", map->current_occupation);
    for (int i=0; i < map->total_size; i++){
        //printf("i%d: %s\n", i, map->hash_map[i].key);
        if (map->hash_map[i].key != lru_null_value && map->hash_map[i].value != NULL){
            //printf("resize key %s\n", map->hash_map[i].key);
            lru_add_key_resize(new_map, map->hash_map[i].key, map->hash_map[i].value);
        }
    }
    map->hash_map = new_map->hash_map;
    map->current_occupation = new_map->current_occupation;
    map->total_size = new_map->total_size;
    return map;
}
int lru_add_key(lru_static_hash_map *map, char *key, link_node *value){
    //printf("Pointer to map add_key %p \n", map);
    if (map->current_occupation == (map->total_size >> 1)){
        //printf("Current occupation %d \n",map->current_occupation);
        map = lru_resize(map);
        //printf("Pointer to map post resize %p \n", map);
    }
    int key_length = strlen(key);
    int summation = 0;
    for (int i = 0; i < key_length; i++){
        summation += (int)key[i];
    }
    int hash_value = lru_hash_function(summation, map->total_size);
    //printf("Hash Function %d \n", hash_value);
    //printf("actual key %s \n", key);
    //printf("Hash key %s \n", map->hash_map[hash_value].key);
    while (map->hash_map[hash_value].key != lru_null_value && map->hash_map[hash_value].value != NULL){
        //printf("KEY %s\n",map->hash_map[hash_value].key);
        //printf("Map total size %d \n", map-> total_size);
        //printf("Current occupation %d \n",map->current_occupation);
        //printf("Total Capacity %d \n",map->total_size);
        //printf("Hash Function %d \n", hash_value);
        if (strcmp(map->hash_map[hash_value].key, key)==0){
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
    lru_map_item mapping;
    mapping.key = strdup(key);
    mapping.value = value;

    map->hash_map[hash_value] = mapping;
    map->current_occupation ++;
    return 0;
}

link_node* lru_get_value(lru_static_hash_map* map, char * key){
    //printf("Key: %s\n", key);
    int key_length = strlen(key);
    int summation = 0;
    for (int i = 0; i < key_length; i++){
        summation += (int)key[i];
    }
    int hash_value = lru_hash_function(summation, map->total_size);
    //printf("Hash Value %d \n", hash_value);
    int original_hash = hash_value;
    while (map->hash_map[hash_value].key != lru_null_value){
        if (hash_value == original_hash -1){
            break;
        }
        if (strcmp(map->hash_map[hash_value].key, key) == 0 && map->hash_map[hash_value].value != NULL){
            return map->hash_map[hash_value].value;
        }
        if (strcmp(map->hash_map[hash_value].key, key) == 0 && map->hash_map[hash_value].value == NULL){
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
int lru_get_size(lru_static_hash_map* map){
    return map->current_occupation;
}
int lru_delete_key(lru_static_hash_map* map, char* key){
    int summation = 0;

    for (int i = 0; i < strlen(key); i++){
        summation += (int) key[i];
    }
    int hash_value = lru_hash_function(summation, map->total_size);
    int original_hash = hash_value;
    while (map->hash_map[hash_value].key != lru_null_value){
        if (hash_value == original_hash -1){
            break;
        }
        if (strcmp(map->hash_map[hash_value].key, key) == 0){
            map->current_occupation -= 1;
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
void lru_print_hash_map(lru_static_hash_map* map){
    //printf("current occupation %d\n", map->current_occupation);
    printf("{");
    int printed_keys = 0;
    for (int i=0; i < map->total_size; i++){
        if (map->hash_map[i].key != lru_null_value){
            printed_keys ++;
            printf("%s:%p", map->hash_map[i].key, map->hash_map[i].value);
            if (printed_keys != map->current_occupation){
                printf(",");
            }
        }
    }
    printf("}\n");
}
void lru_free_memory_hash_map(lru_static_hash_map* map) {
    if (map != NULL) {
        if (map->hash_map != NULL) {
            free(map->hash_map);
            map->hash_map = NULL;
        }
        free(map);
        map = NULL;
    }
}