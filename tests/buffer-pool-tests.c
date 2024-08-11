#include "buffer-pool.h"
int test1(){
    char path[256];
    boot_up_buffer_pool();
    for (int i = 0; i < 12; i++){
        snprintf(path, sizeof(path), "db/%d", i);
        int fd = open(path, O_RDWR|O_CREAT, 0666);
        close(fd);
        memset(path, 0, 256);
        snprintf(path, sizeof(path), "db/%d", i);
        char *location = read_from_buffer_pool(path);
        printf("Pointer to path %p\n", location);
        link_node *node = lru_get_value(manager->lru_hash_map, path);
        node-> pin = 0;
        memset(path, 0, 256);
        print_linked_list();

    }
    memset(path, 0, 256);
    snprintf(path, sizeof(path), "db/%d", 4);
    char *location = read_from_buffer_pool(path);
    printf("Pointer to path %p\n", location);
    link_node *node = lru_get_value(manager->lru_hash_map, path);
    node-> pin = 0;
    print_linked_list();






    /*
    memset(path, 0, 256);
    snprintf(path, sizeof(path), "db/%d", 2);
    char *location = read_from_buffer_pool(path);
    printf("Pointer to path %p\n", location);
    memset(path, 0, 256);
    snprintf(path, sizeof(path), "db/%d", 1);
    location = read_from_buffer_pool(path);
    printf("Pointer to path %p\n", location);
    memset(path, 0, 256);
    snprintf(path, sizeof(path), "db/%d", 2);
    location = read_from_buffer_pool(path);
    printf("Pointer to path %p\n", location);
    */
    return 1;
}



int main(){
    test1();
    //test2();

}