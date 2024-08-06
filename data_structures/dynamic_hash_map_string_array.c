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
char null_value_array[] = "3*0193!~#";
int nan_value = -1111;
static_hash_map_array* construct_hash_map_array();
int hash_function_array(int key,int total_size);
static_hash_map_array* resize_array(static_hash_map_array *map);
int add_key_array(static_hash_map_array* map, char *key, int *value);
int* get_value_array(static_hash_map_array* map, char* key);
int get_size_array(static_hash_map_array* map);
int delete_key_array(static_hash_map_array* map, char* key);
void print_hash_map_array(static_hash_map_array* map);
void free_memory_hash_map_array(static_hash_map_array* map);

static_hash_map_array* construct_hash_map_array(){
    //printf("Construction Called \n");
    static_hash_map_array*map = (static_hash_map_array*)malloc(sizeof(static_hash_map_array));
    //printf("Address: %p\n", map);
    map_item_array mapping;
    int nan_array[2] = {nan_value, nan_value};
    mapping.key = null_value_array;
    mapping.value = nan_array;
    int size = 2;
    map->hash_map = (map_item_array*)malloc(sizeof(map_item_array) * size);
    map->current_occupation = 0;
    map->total_size = size;
    for (int i = 0; i < map->total_size; i++){
        map->hash_map[i] = mapping; 
        //printf("mapping %d : %s\n", i, map->hash_map[i].key);
    }
    //printf("Pointer to map %p \n", map);

    return map;
}
int hash_function_array(int key,int total_size){
    //printf("total size: %d\n", total_size);
    return key% total_size;
}

int add_key_resize_array(static_hash_map_array* new_map, char* key, int* value){
    //printf("Pointer to map add_key %p \n", new_map);
    //printf("total size : %d\n", new_map->total_size);
    int new_key_length = strlen(key);
    int summation = 0;
    for (int i = 0; i < new_key_length; i++){
        summation += (int) key[i];
    }
    int hash_value = hash_function_array(summation, new_map->total_size);
    //printf("Hash Function %d \n", hash_value);
    //printf("Hash key %s \n", new_map->hash_map[hash_value].key);
    while (new_map->hash_map[hash_value].key != null_value_array && new_map->hash_map[hash_value].value[0] != nan_value){
        /*
        printf("KEY %s\n",new_map->hash_map[hash_value].key);
        printf("resize \n");
        printf("Current occupation %d \n",new_map->current_occupation);
        printf("Total Capacity %d \n",new_map->total_size);
        printf("Hash Function %d \n", hash_value);
        */
        if (strcmp(new_map->hash_map[hash_value].key, key) == 0){
            new_map->hash_map[hash_value].value[0] = value[0];
            new_map->hash_map[hash_value].value[1] = value[1];
            return 0;
        }
        if (hash_value + 1 == new_map->total_size){
            hash_value = 0;
        }
        else{
            hash_value ++;
        }   
        }
    map_item_array mapping;
    mapping.key = key;
    mapping.value = value;

    new_map->hash_map[hash_value] = mapping;
    new_map->current_occupation ++;
    return 0;
}

static_hash_map_array * resize_array(static_hash_map_array *map){
    //printf("Resize called \n");
    //printf("Pointer to map inside resize %p \n", map);
    int new_size = map->total_size * 2;
    static_hash_map_array *new_map = (static_hash_map_array*)malloc(sizeof(static_hash_map_array));
    new_map->hash_map = (map_item_array*)malloc(sizeof(map_item_array) * new_size);
    new_map->current_occupation = 0;
    new_map->total_size = new_size;
    map_item_array mapping;
    mapping.key = null_value_array;
    int value_array[2] = {nan_value, nan_value};
    mapping.value = value_array;
    for (int i = 0; i < new_map->total_size; i++){
        new_map->hash_map[i] = mapping; 
    }
    //printf("Map current ocu %d\n", map->current_occupation);
    for (int i=0; i < map->total_size; i++){
        //printf("i%d: %s\n", i, map->hash_map[i].key);
        if (map->hash_map[i].key != null_value_array && map->hash_map[i].value[0] != nan_value){
            //printf("resize key %s\n", map->hash_map[i].key);
            add_key_resize_array(new_map, map->hash_map[i].key, map->hash_map[i].value);
        }
    }
    map->hash_map = new_map->hash_map;
    map->current_occupation = new_map->current_occupation;
    map->total_size = new_map->total_size;
    return map;
}
int add_key_array(static_hash_map_array *map, char *key, int *value){
    //printf("Pointer to map add_key %p \n", map);
    if (map->current_occupation == (map->total_size >> 1)){
        //printf("Current occupation %d \n",map->current_occupation);
        map = resize_array(map);
        //printf("Pointer to map post resize %p \n", map);
    }
    int key_length = strlen(key);
    int summation = 0;
    for (int i = 0; i < key_length; i++){
        summation += (int)key[i];
    }
    int hash_value = hash_function_array(summation, map->total_size);
    /*
    printf("Hash Function %d \n", hash_value);
    printf("actual key %s \n", key);
    printf("Hash key %s \n", map->hash_map[hash_value].key);
    */
    while (map->hash_map[hash_value].key != null_value_array && map->hash_map[hash_value].value[0] != nan_value){
        /*
        printf("KEY %s\n",map->hash_map[hash_value].key);
        printf("Map total size %d \n", map-> total_size);
        printf("Current occupation %d \n",map->current_occupation);
        printf("Total Capacity %d \n",map->total_size);
        printf("Hash Function %d \n", hash_value);
        */
        if (strcmp(map->hash_map[hash_value].key, key)==0){
            printf("Same Key %s\n", key);
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
    map_item_array mapping;
    mapping.key = key;
    mapping.value = value;

    map->hash_map[hash_value] = mapping;
    map->current_occupation ++;
    return 0;
}

int * get_value_array(static_hash_map_array* map, char * key){
    printf("Key: %s\n", key);
    int key_length = strlen(key);
    int summation = 0;
    for (int i = 0; i < key_length; i++){
        summation += (int)key[i];
    }
    int hash_value = hash_function_array(summation, map->total_size);
    printf("Hash Value %d \n", hash_value);
    int original_hash = hash_value;
    while (map->hash_map[hash_value].key != null_value_array){
        if (hash_value == original_hash -1){
            break;
        }
        if (strcmp(map->hash_map[hash_value].key, key) == 0 && map->hash_map[hash_value].value[0] != nan_value){
            return map->hash_map[hash_value].value;
        }
        if (strcmp(map->hash_map[hash_value].key, key) == 0 && map->hash_map[hash_value].value[0] == nan_value){
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
    static int error_array[2];
    error_array[0] = nan_value;
    error_array[1] = nan_value;
    return error_array;
}
int get_size_array(static_hash_map_array* map){
    return map->current_occupation;
}
int delete_key_array(static_hash_map_array* map, char* key){
    int summation = 0;

    for (int i = 0; i < strlen(key); i++){
        summation += (int) key[i];
    }
    int hash_value = hash_function_array(summation, map->total_size);
    int original_hash = hash_value;
    while (map->hash_map[hash_value].key != null_value_array){
        if (hash_value == original_hash -1){
            break;
        }
        if (strcmp(map->hash_map[hash_value].key, key) == 0){
            map->hash_map[hash_value].value[0] = nan_value;
            map->hash_map[hash_value].value[1] = nan_value;
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
void print_hash_map_array(static_hash_map_array* map){
    printf("current occupation %d\n", map->current_occupation);
    printf("{");
    int printed_keys = 0;
    for (int i=0; i < map->total_size; i++){
        if (map->hash_map[i].key != null_value_array){
            printed_keys ++;
            printf("%s:%d,%d", map->hash_map[i].key, map->hash_map[i].value[0], map->hash_map[i].value[1]);
            if (printed_keys != map->current_occupation){
                printf(",");
            }
        }
    }
    printf("}\n");
}
void free_memory_hash_map_array(static_hash_map_array* map) {
    if (map != NULL) {
        if (map->hash_map != NULL) {
            free(map->hash_map);
            map->hash_map = NULL;
        }
        free(map);
        map = NULL;
    }
}