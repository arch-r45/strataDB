#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "a_master_map.h"
#include <stdlib.h>
int test1(){
    static_hash_map_array *map = construct_hash_map_array();
    char key1 [] = "Shea";
    int *arr = (int *)malloc(2 * sizeof(int));
    arr[0] = 73;
    arr[1] = 2025;
    add_key_array(map, key1, arr);

    master_hash_map_array *master_map = master_construct_hash_map_array();
    master_add_key_array(master_map, 1, map);
    static_hash_map_array *new_map = master_get_value_array(master_map, 1);
    int *return_value = get_value_array(new_map, key1);
    printf("return value: %d\n", return_value[1]);

    static_hash_map_array *map2 = construct_hash_map_array();
    char key2 [] = "Rogan";
    int *arr2 = (int *)malloc(2 * sizeof(int));
    arr2[0] = 73;
    arr2[1] = 2022;
    add_key_array(map2, key2, arr2);
    master_add_key_array(master_map, 2, map2);
    new_map = master_get_value_array(master_map, 2);
    return_value = get_value_array(new_map, key2);
    printf("return value: %d\n", return_value[1]);

    master_delete_key_array(master_map, 1);
    new_map = master_get_value_array(master_map, 2);
    return_value = get_value_array(new_map, key2);
    printf("return value: %d\n", return_value[1]);

    new_map = master_get_value_array(master_map, 1);
    if (new_map == NULL){
        printf("Null value\n");
    }

    return 0;
}

int main(){
    test1();
    return 0;
}






