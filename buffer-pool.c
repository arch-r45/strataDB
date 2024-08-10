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

void boot_up_buffer_pool(){
    manager = (buffer_pool*)malloc(sizeof(buffer_pool));
    manager->block = (char*)malloc(sizeof(char) * (manager->capacity * manager->frame_size));
    manager->capacity = 10;
    manager -> lru_hash_map = lru_construct_hash_map();
    manager->free_frame = (int*)malloc(sizeof(int) * manager->capacity);
    memset(manager->free_frame, 0, manager->capacity);
    manager->frame_size = 4000;
    manager-> page_table = NULL;
    lru_cache = (doubly_linked_list*)malloc(sizeof(doubly_linked_list));
    lru_cache->head = (link_node*)malloc(sizeof(link_node));
    lru_cache->tail = (link_node*)malloc(sizeof(link_node));
    char head[] = "0";
    char tail[] = "0";
    lru_cache->head->value = head;
    lru_cache->tail-> value = tail;
    lru_cache->head->next = lru_cache->tail;
    lru_cache->tail->prev = lru_cache->head;
    lru_cache->head->prev = NULL;
    lru_cache->tail->next = NULL;
}
char * read_from_buffer_pool(char *path){
    if (table_get_value(manager->page_table, path) == NULL){
        //1. check the free_frame list for a free slot
        for (int i = 0; i < manager->capacity; i++){
            if (manager->free_frame[i] == 0){
                manager->free_frame[i] = 1;
                table_add_key(&manager -> page_table, path, &manager->block[i*manager->frame_size]);
                int fd = open(path, O_RDONLY, 0);
                link_node *node = (link_node*)malloc(sizeof(link_node));
                node->value = strdup(path);
                node->pin = 1;
                lru_add_key(manager->lru_hash_map, path, node);
                link_node *temp = lru_cache->head->next;
                lru_cache->head->next = node;
                node->prev = lru_cache->head;
                node->next = temp;
                temp->prev = node;
                char *location = table_get_value(manager-> page_table, path);
                read(fd, location, manager->frame_size);
                return location;
            }
        }
        link_node *node = eviction();
        char *location = table_get_value(manager->page_table, node->value);
        printf("location\n");
        memset(location, 0, manager->frame_size);
        node->value = strdup(path);
        printf("node -value %s\n", node->value);
        node->pin = 1;
        printf("node pin%d\n", node->pin);
        table_add_key(&manager -> page_table, path, location);
        printf("added key\n");
        int fd = open(path, O_RDONLY, 0);
        printf("fd\n");
        lru_add_key(manager->lru_hash_map, path, node);
        link_node *temp = lru_cache->head->next;
        printf("temp\n");
        lru_cache->head->next = node;
        node->prev = lru_cache->head;
        node->next = temp;
        temp->prev = node;
        read(fd, location, manager->frame_size);
        return location;
    }
    else{
        char * location = table_get_value(manager-> page_table, path);
        link_node *node = lru_get_value(manager->lru_hash_map, path);
        node->pin = 1;
        link_node *next = node->next;
        link_node *prev = node->prev;
        prev->next = next;
        next->prev = prev;
        link_node * temp = lru_cache->head->next;
        lru_cache->head->next = node;
        node->prev = lru_cache->head;
        node->next = temp;
        temp->prev = node;
        return location;
    }
}
link_node* eviction(){
    link_node *node = lru_cache->tail->prev;
    printf("pointer to Node: %p\n", node);
    printf("pointer to Node -> value: %s\n", node->value);
    printf("pointer to Node -> pin: %d\n", node->pin);
    while (node->pin != 0){
        node = node->prev;
    }
    printf("pointer to Node -> value: %s\n", node->value);
    link_node *prev = node-> prev;
    link_node *next = node-> next;
    node->next = NULL;
    node->prev = NULL;
    prev->next = next;
    next->prev = prev;
    return node;
}
int table_add_key(HashEntry **page_table, char *path, char *block_ptr) {
    printf("add key Called: \n");
    HashEntry *entry = (HashEntry *)malloc(sizeof(HashEntry));
    if (entry == NULL) {
    printf("Failed to allocate memory for entry\n");
    }
    printf("Page: %p\n", page_table);
    printf("entry: %p\n", entry);
    entry->path = strdup(path);
    printf("Entry -> path %s\n", entry->path);
    entry->block_ptr = block_ptr;
    printf("Entry -> block %p\n", entry->block_ptr);
    HASH_ADD_STR(*page_table, path, entry);
    printf("worked\n");
    return 0;
}
char* table_get_value(HashEntry *page_table, char *path) {
    HashEntry *entry;
    HASH_FIND_STR(page_table, path, entry);
    if (entry) {
        return entry->block_ptr;
    } else {
        return NULL;
    }
}
void print_linked_list(){
    link_node * node = lru_cache->head;
    node = node->next;
    while (node != lru_cache->tail){
        printf("string(path) value: %s \n", node->value);
        node = node->next;
    }
}
void free_buffer_pool(){
    memset(manager->block, 0, manager->capacity * manager->frame_size);
    free(manager->block);
    free(manager->free_frame);
    free(manager);
}
/*
int write_to_buffer_pool(char * path){
    if (strcmp(get_value(manager->page_table, path), "None") == 0){
    }
    else{
    }
}
*/