#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
typedef struct {
    int key;
    int value;
}map_item;

typedef struct{
    map_item * hash_map;
    int current_occupation;
    int total_size;
} static_hash_map;
static_hash_map construct_hash_map(int size){
    static_hash_map map;
    map_item mapping;
    mapping.key = -1;
    mapping.value = -1;
    map.hash_map = (map_item*)malloc(sizeof(int) * size);
    map.current_occupation = 0;
    map.total_size = size;
    for (int i = 0; i < map.total_size; i++){
        map.hash_map[i] = mapping; 
    }
    return map;
} 
int hash_function(int key,int total_size){
    return (total_size-1) % key;
}
int add_key(static_hash_map &map, int key, int value){
    if (map.current_occupation == map.total_size){
        return -1;
    }
    int hash_value = hash_function(key, map.total_size);
    printf("Hash Function %d \n", hash_value);
    printf("actual key %d \n", key);
    printf("Hash key %d \n", map.hash_map[hash_value].key);

    while (map.hash_map[hash_value].key != -1 & map.hash_map[hash_value].value != -1){
        printf("Hash Function %d \n", hash_value);
        if (hash_value + 1 == map.total_size){
            hash_value = 0;
        }
        else{
            hash_value ++;
        }   
        }
    map_item mapping;
    mapping.key = key;
    mapping.value = value;

    map.hash_map[hash_value] = mapping;
    return 0;
}
int get_value(static_hash_map &map, int key){
    int hash_value = hash_function(key, map.total_size);
    int original_hash = hash_value;
    while (map.hash_map[hash_value].key != -1 & map.hash_map[hash_value].value != -1){
        if (hash_value == original_hash -1){
            return -1;
        }
        if (map.hash_map[hash_value].key == key){
            return map.hash_map[hash_value].value;
        }
        if (hash_value + 1 == map.total_size){
            hash_value = 0;
        }
        else{
            hash_value ++;
        }   
        }
    return -1;
}

void print_hash_map(static_hash_map &map){
    printf("{");
    for (int i=0; i < map.total_size; i++){
        if (map.hash_map[i].key != -1){
            printf(",");
            printf("%d: %d", map.hash_map[i].key, map.hash_map[i].value);
        }
    }
    printf("}\n");
}
int main(){
    static_hash_map map = construct_hash_map(10);
    add_key(map, 1, 3);
    add_key(map, 4, 9);
    add_key(map, 55, 2018);
    add_key(map, 17, 90);
    add_key(map, 2011121, 3);
    print_hash_map(map);

    int val = get_value(map, 2011121);
    printf("val %d \n", val);
    val = get_value(map, 4);
    printf("val %d \n", val);
    free(map.hash_map);
}


