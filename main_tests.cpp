#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <unordered_map>
#include "main.h"
int test1(){
    int failure = 1;
    construct_hash_map_from_directory();
    //set(char * key, char * value, std::unordered_map<std::string, int*> &map, int file_number, bool compaction)
    char key1[] = "Shea";
    char value1[] = "2013";
    set(key1, value1, master_map[directory_buffer[current_fd_buffer_index]], directory_buffer[current_fd_buffer_index], false);
    char key2[] = "Shea";
    char value2[] = "2014";
    set(key2, value2, master_map[directory_buffer[current_fd_buffer_index]], directory_buffer[current_fd_buffer_index], false);
    std::string return_value = get(key2, directory_buffer, current_fd_buffer_index);
    if (return_value != "2014"){
        printf("Return Value = %s instead of %s \n", return_value.c_str(), value2);
        failure = 0;
    }
    if (current_fd_buffer_index != 0){
        printf("Current Buffer index Wrong: %d\n", current_fd_buffer_index);
    }
    return failure;
}
int test2(){
    construct_hash_map_from_directory();
    int failure = 1;
    char key[] = "Shea";
    int length;
    for (int i = 2013; i < 2018; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        set(key, val, master_map[directory_buffer[current_fd_buffer_index]], directory_buffer[current_fd_buffer_index], false);
        free(val);
    }
    std::string return_value = get(key, directory_buffer, current_fd_buffer_index);
    if (return_value != "2017"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "2017");
        
    }
    char key2[] = "Rogan";
    for (int i = 2013; i < 2018; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        set(key2, val, master_map[directory_buffer[current_fd_buffer_index]], directory_buffer[current_fd_buffer_index], false);
        free(val);

    }
    return_value = get(key, directory_buffer, current_fd_buffer_index);
    if (return_value != "2017"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "2017");
        
    }
    return_value = get(key2, directory_buffer, current_fd_buffer_index);
    if (return_value != "2017"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "2017");
    }
    for (int i = 2020; i < 2026; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        set(key, val, master_map[directory_buffer[current_fd_buffer_index]], directory_buffer[current_fd_buffer_index], false);
        free(val);
    }
    return_value = get(key, directory_buffer, current_fd_buffer_index);
    if (return_value != "2025"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "2017");
    }
    return failure;
}
int main(){
    int total = 0;
    int passed = 0;
    int result = test2();
    total++;
    passed += result;
    printf("Total tests passed: %d, percentage passed: %.2f%%\n", passed, 100.0 * ((float)passed / total));

}