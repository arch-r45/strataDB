#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "data_structures/dynamic_hash_map_string.h"
#include "data_structures/a_master_map.h"
#include <pthread.h>
#include "uthash.h"
#include "data_structures/lru_hash_map.h"
//#include "data_structures/dynamic_hash_map_string_array.h"
int construct_hash_map_from_directory();
char *get(char *key);
int set(char * key, char * value);
void *compaction(void *arg);
int construct_hash_map_from_directory();
int command_line_interface();
void flush_db();
extern int PAGE_FAULT;
extern char tombstone[];
extern int FILE_LIMIT;
extern int directory_buffer[1024];
extern int current_fd_buffer_index;
extern size_t dir_byte_count;
extern int dir_fd;
extern master_hash_map_array master_map;
extern int bit_flag;
typedef struct {
    char *path;              
    char *block_ptr;           
    UT_hash_handle hh;         
} HashEntry;
typedef struct{
    link_node* head;
    link_node* tail;
}doubly_linked_list;
typedef struct{
    int capacity; //this should equal 10
    int frame_size;
    HashEntry *page_table; //Page table -> fixed size maps file path(str) to pointer
    char *block; //call malloc at boot up and grab 40,000 bytes
    int * free_frame;
    lru_static_hash_map* lru_hash_map;
} buffer_pool;
char * read_from_buffer_pool(char *path);
link_node* eviction();
buffer_pool*manager;
doubly_linked_list *lru_cache;
void print_linked_list();
int table_add_key(HashEntry **page_table, char *path, char *block_ptr);
char* table_get_value(HashEntry *page_table, char *path);
void boot_up_buffer_pool();
void free_buffer_pool();
void change_buffer();