#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "dynamic_hash_map_string_array.h"
#include <stdlib.h>

int test1(){
    int failure = 1;
    static_hash_map *map = construct_hash_map_array();
    char key [] = "Rogan";
    int length;
    for (int i = 2013; i < 2018; i++){
        int val[2] = {i, i+1};
        add_key_array(map, key, val);
    }
    int * return_value = get_value_array(map, key);
    printf("return value index 0: %d -> 1: %d\n", return_value[0], return_value[1]);
    char key2 [] = "James";
    int value2[2] = {23, 26};
    add_key_array(map, key2, value2);
    char key3[] = "Jones";
    int value3[2] = {22200201, 2621};
    add_key_array(map, key3, value3);
    delete_key_array(map, key);
    return_value = get_value_array(map, key);
    printf("return value index 0: %d -> 1: %d\n", return_value[0], return_value[1]);
    return_value = get_value_array(map, key2);
    printf("return value index 0: %d -> 1: %d\n", return_value[0], return_value[1]);
    return_value = get_value_array(map, key3);
    printf("return value index 0: %d -> 1: %d\n", return_value[0], return_value[1]);
    return 0;
}
int main(){
    test1();
}