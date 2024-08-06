#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <unordered_map>
#include <time.h>
#include "main.h"
int test1(){
    int failure = 1;
    construct_hash_map_from_directory();
    //set(char * key, char * value, std::unordered_map<std::string, int*> &map, int file_number, bool compaction)
    char key1[] = "Shea";
    char value1[] = "2013";
    set(key1, value1);
    char key2[] = "Shea";
    char value2[] = "2014";
    set(key2, value2);
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
        set(key, val);
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
        set(key2, val);
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
        printf("i %d\n", i);
        printf("current file number %d \n", directory_buffer[current_fd_buffer_index]);
        set(key, val);
        free(val);
    }
    printf("Dictionary %lu\n", master_map[5].count("Shea"));
    printf("Dictionary %lu\n", master_map[6].count("Shea"));
    return_value = get(key, directory_buffer, current_fd_buffer_index);
    if (return_value != "2025"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "2025");
    }
    if (directory_buffer[current_fd_buffer_index] != 6){
        failure = 0;
        printf("Current File = %d not %d \n", directory_buffer[current_fd_buffer_index], 6);
    }
    return failure;
}
int test3(){
    construct_hash_map_from_directory();
    int failure = 1;
    FILE_LIMIT = 10;
    PAGE_FAULT = 1024;
    char key1[] = "Shea";
    int length;
    for (int i = 1834; i < 2025; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        set(key1, val);
        free(val);
    }
    char key2[] = "Rogan";
    for (int i = 1834; i < 2031; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        set(key2, val);
        free(val);
    }
    char key3[] = "Shakespeare";
    for (int i = 1564; i < 1617; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        set(key3, val);
        free(val);
    }
    std::string return_value = get(key1, directory_buffer, current_fd_buffer_index);
    if (return_value != "2024"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "2024");
    }
    return_value = get(key2, directory_buffer, current_fd_buffer_index);
    if (return_value != "2030"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "2030");
    }
    return_value = get(key3, directory_buffer, current_fd_buffer_index);
    if (return_value != "1616"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "1616");
    }
    char key4 [] = "Knuth";
    return_value = get(key4, directory_buffer, current_fd_buffer_index);
    if (return_value != "Key Does Not Exist"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "Key Does Not Exist");
    }
    return failure;
}
int test4(){
    construct_hash_map_from_directory();
    int failure = 1;
    // changing file limit to 5 here allows compaction to be called
    FILE_LIMIT = 5;
    PAGE_FAULT = 1024;
    char key1[] = "Shea";
    int length;
    for (int i = 1834; i < 2025; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        set(key1, val);
        free(val);
    }
    char key2[] = "Rogan";
    for (int i = 1834; i < 2031; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        set(key2, val);
        free(val);
    }
    char key3[] = "Shakespeare";
    for (int i = 1564; i < 1617; i++){
        length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        set(key3, val);
        free(val);
    }
    std::string return_value = get(key1, directory_buffer, current_fd_buffer_index);
    if (return_value != "2024"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "2024");
    }
    return_value = get(key2, directory_buffer, current_fd_buffer_index);
    if (return_value != "2030"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "2030");
    }
    return_value = get(key3, directory_buffer, current_fd_buffer_index);
    if (return_value != "1616"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "1616");
    }
    char key4 [] = "Knuth";
    return_value = get(key4, directory_buffer, current_fd_buffer_index);
    if (return_value != "Key Does Not Exist"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "Key Does Not Exist");
    }
    return failure;
}
int test5(){
    //stress test and time limit - no compaction -> Im oscilating between the three keys to simulate a real env
    PAGE_FAULT = 10000;
    FILE_LIMIT = 10;
    int failure = 1;
    construct_hash_map_from_directory();
    char key1[] = "Shea";
    char key2[] = "Rogan";
    char key3[] = "Shakespeare";
    clock_t start = clock();
    int upper_bound = 6000;
    for (int i = 0; i < upper_bound; i++){
        int length = snprintf(NULL, 0, "%d", i);
        char *val = (char*)malloc(length+1);
        snprintf(val, length + 1, "%d", i);
        if (i % 3 == 0){
            set(key1, val);
        }
        else if (i % 3 == 1){
            set(key2, val);
        }
        else{
            set(key3, val);
        }
    }
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Number of Writes: %d writes\nTime taken: %f seconds\n", upper_bound, time_taken);
    std::string return_value = get(key1, directory_buffer, current_fd_buffer_index);
    if (return_value != "5997"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "5997");
    }
    return_value = get(key2, directory_buffer, current_fd_buffer_index);
    if (return_value != "5998"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "5998");
    }
    return_value = get(key3, directory_buffer, current_fd_buffer_index);
    if (return_value != "5999"){
        failure = 0;
        printf("Return Value = %s instead of %s \n", return_value.c_str(), "5999");
    }
    return failure; 
}
int main(){
    int total = 0;
    int passed = 0;
    int result = 0;

    result = test1();
    total ++;
    if (result == 0){
        printf("TEST FAILED NUMBER %d \n", total);
    }
    passed += result;
    /*
    flush_db();
    result = test2();
    total++;
    if (result == 0){
        printf("TEST FAILED NUMBER %d \n", total);
    }
    passed += result;
    flush_db();

    result = test3();
    total ++;
    if (result == 0){
        printf("TEST FAILED NUMBER %d \n", total);
    }
    passed += result;
    flush_db();
    result = test4();
    total ++;
    if (result == 0){
        printf("TEST FAILED NUMBER %d \n", total);
    }
    passed += result;
    flush_db();
    result = test5();
    total ++;
    if (result == 0){
        printf("TEST FAILED NUMBER %d \n", total);
    }
    passed += result;
    */
    printf("Total tests passed: %d, percentage passed: %.2f%%\n", passed, 100.0 * ((float)passed / total));
}
