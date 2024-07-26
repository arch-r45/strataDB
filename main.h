#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <unordered_map>
int construct_hash_map_from_directory();
std::string get(char *key, int * directory_buffer, int current_fd_buffer_index);
void check_page_fault();
int set(char * key, char * value, std::unordered_map<std::string, int*> &map, int file_number, bool compaction);
void compaction(int *directory_buffer, int &current_fd_buffer_index, int dir_fd, size_t directory_buffer_size);
int construct_hash_map_from_directory();
int command_line_interface();
extern int PAGE_FAULT;
extern char tombstone[];
extern int FILE_LIMIT;
extern int directory_buffer[1024];
extern int current_fd_buffer_index;
extern size_t dir_byte_count;
extern int dir_fd;
extern std::unordered_map<int, std::unordered_map<std::string, int*>> master_map;