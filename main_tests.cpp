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


int main(){
    int total = 0;
    int passed = 0;
    int result = test1();
    total++;
    passed += result;
    printf("Total tests passed: %d, percentage passed: %.2f%%\n", passed, 100.0 * ((float)passed / total));

}