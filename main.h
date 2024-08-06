#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <unordered_map>
#include "data_structures/dynamic_hash_map_string.h"
#include "data_structures/dynamic_hash_map_string_array.h"
int construct_hash_map_from_directory();
std::string get(char *key, int * directory_buffer, int current_fd_buffer_index);
void check_page_fault();
int set(char * key, char * value);
void compaction(int *directory_buffer, int &current_fd_buffer_index, int dir_fd, size_t directory_buffer_size);
int construct_hash_map_from_directory();
int command_line_interface();
int setter_for_compaction(char * key, char * value, static_hash_map_array*map, int file_number);
void flush_db();
extern int PAGE_FAULT;
extern char tombstone[];
extern int FILE_LIMIT;
extern int directory_buffer[1024];
extern int current_fd_buffer_index;
extern size_t dir_byte_count;
extern int dir_fd;
extern std::unordered_map<int, std::unordered_map<std::string, int*>> master_map;