#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "dynamic_hash_map_string.h"
#include <stdlib.h>
int test1(){
    int failure = 1;
    construct_hash_map();
    char key [] = "Rogan";
    int length;
    for (int i = 2013; i < 2018; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        add_key(key, val);
    }
    char * return_value = get_value(key);
    printf("Return Value %s \n", return_value);
    char key2 [] = "Shea";
    for (int i = 2013; i < 2020; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        add_key(key2, val);
    }
    return_value = get_value(key);
    printf("Return Value %s \n", return_value);
    return_value = get_value(key2);
    printf("Return Value %s \n", return_value);
    print_hash_map();
    return failure;
}
void run_tests(){
    int passed = 0;
    int total = 0;
    int result;
    result = test1();
}
int main(){
    printf("Started");
    run_tests();
    return 1;
}


