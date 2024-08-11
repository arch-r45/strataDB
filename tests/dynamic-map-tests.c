#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "dynamic_hash_map.h"
#include <stdlib.h>
int test1(){
    int failure = 1;
    construct_hash_map();
    //set, set, set, get, get
    add_key(3, 4);
    print_hash_map(map);
    add_key(2, 9);
    add_key(10, 20);
    int size = get_size(map);
    printf("Size == %d\n", size);
    if (size != 3){
        failure = 0;
    }
    int val = get_value(2);
    printf("Value == %d \n", val);
    if (val != 9){
        failure = 0;
    }
    free_memory_hash_map();
    return failure;

}
int test2(){
    construct_hash_map();
    int failure = 1;
    add_key(1, 3);
    add_key(4, 9);
    add_key(55, 2018);
    add_key(17, 90);
    printf("Current occupation %d\n", map->current_occupation);
    print_hash_map();
    add_key(2011121, 3);

    int val = get_value(2011121);
    if (val != 3){
        failure = 0;
        printf("val %d does not equal key %d \n", 2011121, 3);
        printf("val %d \n", val);
    }
    val = get_value(4);
    if (val != 9){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 9);
    }
    printf("val %d \n", val);
    val = get_size();
    if (val != 5){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 5);
    }
    printf("Size %d \n", val);
    printf("total capacity %d\n", map->total_size);
    delete_key(1);
    delete_key(4);
    val = get_value(4);
    if (val != -1){
        failure = 0;
        printf("key: %d not deleted  \n", 4);
    }
    val = get_size(map);
    printf("Val: %d\n", val);
    free_memory_hash_map();
    return failure;
}
int test3(){
    construct_hash_map();
    int failure = 1;
    add_key(2, 8);
    add_key(8, 64);
    add_key(102, 204);
    add_key(56, 112);
    print_hash_map();

    int val = get_value(2);
    if (val != 8){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 8);
    }
    printf("val %d \n", val);
    val = get_value(102);
    if (val != 204){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 204);
    }
    printf("val %d \n", val);
    val = get_size();
    if (val != 4){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 4);
    }
    printf("Size %d \n", val);

    delete_key(8);
    val = get_value(8);
    if (val != -1){
        failure = 0;
        printf("key: %d not deleted  \n", 8);
    }
    val = get_size();
    printf("Val: %d\n", val);
    free_memory_hash_map();

    return failure;
}

int test4(){
    construct_hash_map();
    int failure = 1;
    // Assuming the hash function may cause collision for these keys
    add_key(11, 111);
    add_key(21, 121);
    print_hash_map(map);

    int val = get_value(11);
    if (val != 111){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 111);
    }
    printf("val %d \n", val);

    val = get_value(21);
    if (val != 121){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 121);
    }
    printf("val %d \n", val);

    val = get_size();
    if (val != 2){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 2);
    }
    printf("Size %d \n", val);

    free_memory_hash_map();
    return failure;
}

int test5(){
    construct_hash_map();
    int failure = 1;
    add_key(10, 100);
    add_key(20, 200);
    add_key(30, 300);
    add_key(40, 400);
    add_key(50, 500);
    print_hash_map();

    int val = get_value(30);
    if (val != 300){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 300);
    }
    printf("val %d \n", val);
    val = get_value(10);
    if (val != 100){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 100);
    }
    printf("val %d \n", val);
    val = get_size();
    if (val != 5){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 5);
    }
    printf("Size %d \n", val);

    delete_key(50);
    val = get_value(50);
    if (val != -1){
        failure = 0;
        printf("key: %d not deleted  \n", 50);
    }
    val = get_size();
    printf("Val: %d\n", val);
    free_memory_hash_map();

    return failure;
}
int test6(){
    construct_hash_map();
    int failure = 1;
    add_key(5, 25);
    add_key(5, 50);
    print_hash_map();

    int val = get_value(5);
    if (val != 50){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 50);
    }
    printf("val %d \n", val);

    val = get_size();
    if (val != 1){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 1);
    }
    printf("Size %d \n", val);
    free_memory_hash_map();
    return failure;
}


int test7(){
    printf("test 6 started \n");

    construct_hash_map();
    int failure = 1;
    add_key(7, 14);
    delete_key(20); // Deleting non-existent key
    print_hash_map();

    int val = get_value(7);
    if (val != 14){
        failure = 0;
        printf("val %d does not equal key %d \n", val, 14);
    }
    printf("val %d \n", val);

    val = get_size();
    if (val != 1){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 1);
    }
    printf("Size %d \n", val);

    free_memory_hash_map();

    return failure;
}
int test8(){
    construct_hash_map();
    int failure = 1;
    for (int i = 1; i < 100; i++){
        add_key(i, i*10);
    }
    print_hash_map(map);
    for (int i = 1; i < 100; i++){
        int val = get_value(i);
        if (val != i*10){
            failure = 0;
            printf("val %d does not equal key %d \n", val, i*10);
        }
        printf("val %d \n", val);
    }
    int val = get_size();
    if (val != 99){
        failure = 0;
        printf("val %d does not equal size %d \n", val, 99);
    }
    printf("Size %d \n", val);

    free_memory_hash_map();

    return failure;
}
void run_tests(){
    int passed = 0;
    int total = 0;
    int result;
    result = test1();
    total ++;
    passed += result;
    result = test2();
    total ++;
    passed += result;
    result = test3();
    total ++;
    passed += result;
    result = test4();
    total++;
    passed += result;
    result = test5();
    total ++;
    passed += result;
    result = test6();
    total ++;
    passed += result;
    result = test7();
    total ++;
    passed += result;
    result = test8();
    total ++;
    passed += result;
    printf("Total tests passed: %d, percentage passed: %.2f%%\n", passed, 100.0 * ((float)passed / total));
}
int main(){
    printf("Started");
    run_tests();
    return 1;
}