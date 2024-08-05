#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "dynamic_hash_map_string.h"
#include <stdlib.h>
int test1(){
    int failure = 1;
    static_hash_map *map = construct_hash_map();
    char key [] = "Rogan";
    int length;
    for (int i = 2013; i < 2018; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        add_key(map, key, val);
    }
    char * return_value = get_value(map, key);
    printf("Return Value %s \n", return_value);
    char key2 [] = "Shea";
    for (int i = 2013; i < 2020; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        add_key(map, key2, val);
    }
    return_value = get_value(map, key);
    printf("Return Value %s \n", return_value);
    return_value = get_value(map, key2);
    printf("Return Value %s \n", return_value);
    print_hash_map(map);
    int size = get_size(map);
    printf("size %d\n", size);
    char key3[] = "Jones";
    char val3[] = "2090";
    add_key(map, key3, val3);
    return_value = get_value(map, key3);
    if (strcmp(return_value, "2090") != 0){
        printf("Re %s Does not equal 2090\n", return_value);
        failure = 0;
    }
    printf("Re %s \n", return_value);
    return_value = get_value(map, key);
    if (strcmp(return_value, "2017") != 0){
        printf("Re %s Does not equal 2017\n", return_value);
        failure = 0;
    }
    printf("Re %s \n", return_value);
    return_value = get_value(map, key2);
    if (strcmp(return_value, "2019") != 0){
        printf("Re %s Does not equal 2019\n", return_value);
        failure = 0;
    }
    printf("Re %s \n", return_value);
    return failure;
}

void run_tests(){
    int passed = 0;
    int total = 0;
    int result;
    passed += test1();
    total++;
    printf("Total tests passed: %d, percentage passed: %.2f%%\n", passed, 100.0 * ((float)passed / total));
}
int main(){
    printf("Started");
    run_tests();
    return 1;
}


