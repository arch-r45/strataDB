#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "hash_map.h"
#include <stdlib.h>
int test1(){
    static_hash_map *map= construct_hash_map(10);
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
    val = get_size(map);
    printf("Val: %d\n", val);
    free_memory_hash_map(map);
    return failure;
}


int test2(){
    static_hash_map *map = construct_hash_map(20);
    int failure = 1;
    add_key(map, 2, 8);
    add_key(map, 8, 64);
    add_key(map, 102, 204);
    add_key(map, 56, 112);
    print_hash_map(map);

    int val = get_value(map, 2);
    if (val != 8){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 8);
    }
    printf("val %d \n", val);
    val = get_value(map, 102);
    if (val != 204){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 204);
    }
    printf("val %d \n", val);
    val = get_size(map);
    if (val != 4){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 4);
    }
    printf("Size %d \n", val);

    delete_key(map, 8);
    val = get_value(map, 8);
    if (val != -1){
        failure = 0;
        printf("key: %d not deleted  \n", 8);
    }
    val = get_size(map);
    printf("Val: %d\n", val);
    free_memory_hash_map(map);

    return failure;
}

int test3(){
    static_hash_map* map = construct_hash_map(15);
    int failure = 1;
    add_key(map, 10, 100);
    add_key(map, 20, 200);
    add_key(map, 30, 300);
    add_key(map, 40, 400);
    add_key(map, 50, 500);
    print_hash_map(map);

    int val = get_value(map, 30);
    if (val != 300){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 300);
    }
    printf("val %d \n", val);
    val = get_value(map, 10);
    if (val != 100){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 100);
    }
    printf("val %d \n", val);
    val = get_size(map);
    if (val != 5){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 5);
    }
    printf("Size %d \n", val);

    delete_key(map, 50);
    val = get_value(map, 50);
    if (val != -1){
        failure = 0;
        printf("key: %d not deleted  \n", 50);
    }
    val = get_size(map);
    printf("Val: %d\n", val);
    free_memory_hash_map(map);

    return failure;
}

int test4(){
    static_hash_map* map = construct_hash_map(10);
    int failure = 1;
    // Assuming the hash function may cause collision for these keys
    add_key(map, 11, 111);
    add_key(map, 21, 121);
    print_hash_map(map);

    int val = get_value(map, 11);
    if (val != 111){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 111);
    }
    printf("val %d \n", val);

    val = get_value(map, 21);
    if (val != 121){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 121);
    }
    printf("val %d \n", val);

    val = get_size(map);
    if (val != 2){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 2);
    }
    printf("Size %d \n", val);

    free_memory_hash_map(map);
    return failure;
}

int test5(){
    static_hash_map* map = construct_hash_map(10);
    int failure = 1;
    add_key(map, 5, 25);
    add_key(map, 5, 50);
    print_hash_map(map);

    int val = get_value(map, 5);
    if (val != 50){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 50);
    }
    printf("val %d \n", val);

    val = get_size(map);
    if (val != 1){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 1);
    }
    printf("Size %d \n", val);
    free_memory_hash_map(map);
    return failure;
}
int test6(){
    printf("test 6 started \n");

    static_hash_map* map = construct_hash_map(10);
    int failure = 1;
    add_key(map, 7, 14);
    delete_key(map, 20); // Deleting non-existent key
    print_hash_map(map);

    int val = get_value(map, 7);
    if (val != 14){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 14);
    }
    printf("val %d \n", val);

    val = get_size(map);
    if (val != 1){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 1);
    }
    printf("Size %d \n", val);

    free_memory_hash_map(map);

    return failure;
}
int test7(){
    static_hash_map* map = construct_hash_map(100);
    int failure = 1;
    for (int i = 1; i < 100; i++){
        add_key(map, i, i*10);
    }
    print_hash_map(map);
    for (int i = 1; i < 100; i++){
        int val = get_value(map, i);
        if (val != i*10){
            failure = 0;
            printf("val %d does not equal key %d \n", val, i*10);
        }
        printf("val %d \n", val);
    }
    int val = get_size(map);
    if (val != 99){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 99);
    }
    printf("Size %d \n", val);

    free_memory_hash_map(map);

    return failure;
}




void run_tests(){
    int passed = 0;
    int total = 0;
    int result;
    result = test1();
    passed += result;
    total ++;
    printf("%d\n", total);
    result = test2();
    passed += result;
    total ++;
    printf("%d\n", total);
    result = test3();
    passed += result;
    total++;
    printf("%d\n", total);
    result = test4();
    passed+= result;
    total++;
    printf("%d\n", total);
    result = test5();
    passed += result;
    total++;
    printf("%d\n", total);
    result = test6();
    passed += result;
    total++;
    printf("%d\n", total);
    result = test7();
    passed += result;
    total++;
    printf("%d\n", total);

    printf("Total tests passed: %d, percentage passed: %.2f%%\n", passed, 100.0 * ((float)passed / total));
}

int main(){
    printf("Started");
    run_tests();
    return 1;
}