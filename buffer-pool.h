#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "data_structures/lru_hash_map.h"
#include "uthash.h"


typedef struct {
    char *path;              
    char *block_ptr;           
    UT_hash_handle hh;         
} HashEntry;

typedef struct{
    link_node* head;
    link_node * tail;
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
void boot_up_buffer_pool();
extern buffer_pool*manager;
extern doubly_linked_list *lru_cache;
char* table_get_value(HashEntry *page_table, char *path);
int table_add_key(HashEntry **page_table, char *path, char *block_ptr);
void print_linked_list();
