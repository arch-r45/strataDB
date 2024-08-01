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
    /*
    int size = get_size(map);
    printf("Size == %d\n", size);
    if (size != 3){
        failure = 0;
    }
    */
    int val = get_value(2);
    printf("Value == %d \n", val);
    if (val != 9){
        failure = 0;
    }
    return failure;
}

void run_tests(){
    int passed = 0;
    int total = 0;
    int result;
    result = test1();
    total ++;
    passed += result;
    printf("Total tests passed: %d, percentage passed: %.2f%%\n", passed, 100.0 * ((float)passed / total));
}
int main(){
    printf("Started");
    run_tests();
    return 1;
}