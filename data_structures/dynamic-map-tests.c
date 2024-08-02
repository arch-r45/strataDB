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
    printf("Total tests passed: %d, percentage passed: %.2f%%\n", passed, 100.0 * ((float)passed / total));
}
int main(){
    printf("Started");
    run_tests();
    return 1;
}