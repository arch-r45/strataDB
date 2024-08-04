#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "dynamic_hash_map_string.h"
#include <stdlib.h>
int test1(){
    int failure = 1;
    char key [] = "Rogan";
    char value[] = "2013";
    construct_hash_map();
    add_key(key, value);
    char * return_value = get_value(key);
    printf("Return Value %s \n", return_value);
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


