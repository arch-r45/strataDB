#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include "hash_map.h"
int test1(){
    static_hash_map map = construct_hash_map(10);
    int failure = 1;
    add_key(map, 1, 3);
    add_key(map, 4, 9);
    add_key(map, 55, 2018);
    add_key(map, 17, 90);
    add_key(map, 2011121, 3);
    print_hash_map(map);
    int val = get_value(map, 2011121);
    if (val != 3){
        failure = 0;
        printf("val %d does not equal key %d \n", 2011121, 3);
    }
    printf("val %d \n", val);
    val = get_value(map, 4);
    if (val != 9){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 9);
    }
    printf("val %d \n", val);
    val = get_size(map);
    if (val != 5){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 5);
    }
    printf("Size %d \n", val);
    delete_key(map, 1);
    delete_key(map, 4);
    val = get_value(map, 4);
    if (val != -1){
        failure = 0;
        printf("key: %d not deleted  \n", 4);
    }
    free(map.hash_map);

    return failure;
}


void run_tests(){
    int passed = 0;
    int total = 0;
    int result;
    result = test1();
    passed += result;
    total ++;

    printf("Total tests passed:%d, percentage passed: %f\n", passed, 100*float(passed/total));

}

int main(){
    run_tests();
    return 1;
}