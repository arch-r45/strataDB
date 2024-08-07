#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "data_structures/dynamic_hash_map_string.h"
//#include "data_structures/dynamic_hash_map_string_array.h"
#include "data_structures/a_master_map.h"
#include <pthread.h>
int construct_hash_map_from_directory();
int setter_for_compaction(char * key, char * value, static_hash_map_array*map, int file_number);
char *get(char *key);
void check_page_fault();
int set(char * key, char * value);
void *compaction(void *arg);
int construct_hash_map_from_directory();
int command_line_interface();
void flush_db();
int PAGE_FAULT = 40;
char tombstone[] = "_TOMBSTONE_";
int FILE_LIMIT = 3;
int directory_buffer [1024];
int current_fd_buffer_index = -1;
size_t dir_byte_count;
int dir_fd;
int bit_flag = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
//std::unordered_map<int, static_hash_map_array*> master_map;
master_hash_map_array *master_map;
char *get(char *key){
    int locked = 0;
    int file_index = -1;
    //printf("Current File Index %d \n", current_fd_buffer_index);
    //printf("current Fd Buffer Index %d\n", current_fd_buffer_index);
    pthread_mutex_lock(&mtx);
    for (int i = current_fd_buffer_index; i > -1; i --){
        //master_map[directory_buffer[i]
        int *temp_array = get_value_array(master_get_value_array(master_map, directory_buffer[i]), key);
        if (temp_array[0] != nan_value){
            file_index = directory_buffer[i];
            break;
        }
    }
    if (file_index == -1){
        pthread_mutex_unlock(&mtx);
        return "Key Does Not Exist";
        
    }
    //printf("File Index of key %s : --> db/%d \n", key, file_index);
    char path[256];
    snprintf(path, sizeof(path), "db/%d", file_index);
    int fd = open(path, O_RDONLY, 0);
    //printf("opening file %d", fd);
    memset(path, 0, 256);
    //master_map[file_index]
    int *arr = get_value_array(master_get_value_array(master_map, file_index), key);
    int offset = arr[0];
    int length_of_record = arr[1];
    //printf("offset: %d and Length of record: %d\n", offset, length_of_record);
    char * current_file_buf;
    current_file_buf = (char*)malloc((length_of_record + 1) * sizeof(char));
    lseek(fd, offset, 0);
    int bytes_read = read(fd, current_file_buf, length_of_record);
    if (bytes_read != length_of_record){
        //printf("Mismatch between Bytes Read: %d and Length of Record %d \n", bytes_read, length_of_record);
    }
    int size_of_key;
    int size_of_value;
    memcpy(&size_of_key, current_file_buf, sizeof(int));
    memcpy(&size_of_value, current_file_buf+ sizeof(int), sizeof(int));
    char *found_key;
    found_key = (char*) malloc ((size_of_key + 1) * sizeof(char));
    memcpy(found_key, current_file_buf + sizeof(int) + sizeof(int), size_of_key);
    found_key[size_of_key] = '\0';
    //printf("Found Key: %s, key: %s\n", found_key, key);
    if (strcmp(found_key, key) != 0){
        //printf("Found key %s is not equal to inputted key: %s ", found_key, key);
        pthread_mutex_unlock(&mtx);
        return "Keys are not equal in DB";
    }
    char *found_value;
    found_value = (char*) malloc((size_of_value + 1) * sizeof(char));
    memcpy(found_value, current_file_buf + sizeof(int) + sizeof(int) + size_of_key, size_of_value);
    found_value[size_of_value] = '\0';
    if (strcmp(tombstone, found_value) == 0){
        pthread_mutex_unlock(&mtx);
        return "Key Does Not Exist";

    }
    pthread_mutex_unlock(&mtx);
    free(found_key);
    //free(found_value);
    free(current_file_buf);
    //std::string s(found_value);
    return found_value;
}
void check_page_fault(){
    char current_file_buffer [1024];
    int fd;
    char path[256];
    snprintf(path, sizeof(path), "db/%d", directory_buffer[current_fd_buffer_index]);
    fd = open(path, O_RDWR|O_CREAT, 0666);
    lseek(fd, 0L, 0);
    int size_in_bytes = read(fd, current_file_buffer, 1024);
    //printf("size_in_bytes of file %d \n", size_in_bytes);
    //printf("Size of Bytes %d --> Page Fault %d \n", size_in_bytes, PAGE_FAULT);
    if (size_in_bytes >= PAGE_FAULT){
        //printf("Changing Page\n");
        close(fd);
        int *copy = malloc(sizeof(int));
        *copy = current_fd_buffer_index;
        //printf("copy: %d\n", *copy);
        directory_buffer[current_fd_buffer_index + 1] = directory_buffer[current_fd_buffer_index]+1;
        current_fd_buffer_index ++;
        /*
        The last file is now essentially immutable, it will never be written to again
        Only read from
        */
        //printf("Current Fd_buffer_index: %d \n", current_fd_buffer_index);

        //printf("New Fd_buffer_index: %d \n", current_fd_buffer_index);
        dir_byte_count = sizeof(directory_buffer);
        lseek(dir_fd, 0L, 2);
        int new_bytes = write(dir_fd, &directory_buffer[current_fd_buffer_index],(sizeof(int)));
        //printf("new_bytes %d \n", new_bytes);
        char path[256];
        snprintf(path, sizeof(path), "db/%d", directory_buffer[current_fd_buffer_index]);
        /*
        for (int p = 0; p < current_fd_buffer_index+1; p++){
            //printf("index: %d, file number: %d \n", p, directory_buffer[p]);
        }
        */
        int fd = open(path, O_RDWR|O_CREAT, 0666);
        close(fd);
        //printf("file fd %d \n", fd);
        memset(path, 0, 256);
        //std::unordered_map<std::string, int*> &map = master_map[directory_buffer[current_fd_buffer_index]];
        static_hash_map_array *map = construct_hash_map_array();
        //master_map[directory_buffer[current_fd_buffer_index]] = map;
        master_add_key_array(master_map, directory_buffer[current_fd_buffer_index], map);
        //printf("File number %d\n", directory_buffer[current_fd_buffer_index]);
        //printf("Pointer to map %p \n", &map);
        pthread_t t1;
        int s;
        if (current_fd_buffer_index > FILE_LIMIT){
            bit_flag = 1;
            s = pthread_create(&t1, NULL, compaction, copy);
            pthread_detach(t1);
            //compaction(directory_buffer, copy);
            //printf("Current Fd File Index Outside of compaction %d \n", current_fd_buffer_index);
        }
    }

}
int set(char * key, char * value){
    //printf("key %s\n", key);
    //printf("Value %s \n", value);
    if (bit_flag == 0){
        check_page_fault();
    }
    int file_number;
    pthread_mutex_lock(&mtx);
    if (bit_flag == 1){

        file_number = directory_buffer[current_fd_buffer_index]; 
    }
    else{
        file_number = directory_buffer[current_fd_buffer_index];
    }
    pthread_mutex_unlock(&mtx);
    char path[256];
    snprintf(path, sizeof(path), "db/%d", file_number);  
    int fd = open(path, O_RDWR|O_CREAT, 0666);
    //printf("file path %d \n", file_number);
    memset(path, 0, 256);
    int PAGE_SIZE = 4096;
    char buf[PAGE_SIZE];
    int *arr = (int *)malloc(2 * sizeof(int));
    int key_length = strlen(key);
    int val_length = strlen(value);
    memset(buf, 0, PAGE_SIZE);
    memcpy(buf, &key_length, sizeof(int));
    memcpy(buf + sizeof(int), &val_length, sizeof(int));
    memcpy(buf + (2 * sizeof(int)), key, key_length);
    memcpy(buf + (2* sizeof(int)) + key_length, value, val_length);
    int total_length = (2 * sizeof(int)) + key_length + val_length;
    buf[total_length] = '\0';
    //printf("Total Length: %d\n", total_length);
    //printf("Size of buf %lu\n", sizeof(buf));
    long pos = lseek(fd, 0L, 2);
    //printf("Byte Offset in File %ld \n", pos);
    int bytes_written;
    bytes_written = write(fd, buf, total_length);
    close(fd);
    //printf("Bytes Written %d \n", bytes_written);
    if (bytes_written == total_length){
        //printf("Success \n");
        //std::string s(key);
        //std::cout << s << "string \n";
        //printf("Address of Pointer to array: %p\n", arr);
        arr[0] = pos;
        //printf("Working\n");
        arr[1] = total_length;
        //printf("Working\n");
        //printf("Array 1 %d \n", arr[1]);
        //master_map[file_number][s] = arr;
        //master_map[file_number]
        add_key_array(master_get_value_array(master_map, file_number), key, arr);
        //printf("Working\n");
        memset(buf, 0, PAGE_SIZE);
        return 0;
    }
    memset(buf, 0, PAGE_SIZE);
    return -1;
}
int setter_for_compaction(char * key, char * value, static_hash_map_array*map, int file_number){
    //printf("key %s\n", key);
    //printf("Value %s \n", value);
    char path[256];
    snprintf(path, sizeof(path), "db/%d", file_number);    
    int fd = open(path, O_RDWR|O_CREAT, 0666);
    //printf("file path %d \n", file_number);
    memset(path, 0, 256);
    int PAGE_SIZE = 4096;
    char buf[PAGE_SIZE];
    int *arr = (int *)malloc(2 * sizeof(int));
    int key_length = strlen(key);
    int val_length = strlen(value);
    memset(buf, 0, PAGE_SIZE);
    memcpy(buf, &key_length, sizeof(int));
    memcpy(buf + sizeof(int), &val_length, sizeof(int));
    memcpy(buf + (2 * sizeof(int)), key, key_length);
    memcpy(buf + (2* sizeof(int)) + key_length, value, val_length);
    int total_length = (2 * sizeof(int)) + key_length + val_length;
    buf[total_length] = '\0';
    //printf("Total Length: %d\n", total_length);
    //printf("Size of buf %lu\n", sizeof(buf));
    long pos = lseek(fd, 0L, 2);
    //printf("Byte Offset in File %ld \n", pos);
    int bytes_written;
    bytes_written = write(fd, buf, total_length);
    close(fd);
    //printf("Bytes Written %d \n", bytes_written);
    if (bytes_written == total_length){
        //printf("Success \n");
        //printf("Address of Pointer to array: %p\n", arr);
        arr[0] = pos;
        //printf("Working\n");
        arr[1] = total_length;
        //printf("Working\n");
        //printf("Array 1 %d \n", arr[1]);
        /*
        if (master_map.find(file_number) == master_map.end()){
            printf("Not Found");
            std::unordered_map<std::string, int*> map;
            master_map[directory_buffer[current_fd_buffer_index]] = map;
        }
        */
        //printf("Pointer to map %p \n", &map);
        add_key_array(map, key, arr);
        //printf("Working\n");
        memset(buf, 0, PAGE_SIZE);
        return 0;
    }
    memset(buf, 0, PAGE_SIZE);
    return -1;
}
void *compaction(void *arg){
    //std::unordered_map<std::string, std::string> temp_map;
    //printf("Compaction Begins\n");
    int copy = *(int *)arg;
    //printf("copy %d\n", copy);
    static_hash_map *temp_map = construct_hash_map();
    char path [256];
    int current_fd_buffer_index_copy = copy;
    int original_buffer_index = copy;
    //printf("Original Buffer at start of compaction %d\n", original_buffer_index);
    int fd;
    char file_buffer [1024];
    int deleted_group [current_fd_buffer_index_copy];
    for (int i = 0; i < current_fd_buffer_index_copy+1; i++){
        snprintf(path, sizeof(path), "db/%d", directory_buffer[i]);
        fd = open(path, O_RDONLY, 0666);
        memset(path, 0, 256);
        lseek(fd, 0L, 0);
        deleted_group[i] = directory_buffer[i];
        int bytes_read = read(fd, file_buffer, 1024);
        int key_size;
        int value_size;
        int j = 0;
        while (j < bytes_read){
            memcpy(&key_size, file_buffer + j, sizeof(int));
            memcpy(&value_size, file_buffer+ j + sizeof(int), sizeof(int));
            char * key = (char*) malloc(key_size+1);
            char * value = (char *) malloc(value_size+1);
            memcpy(key, file_buffer + j + sizeof(int) + sizeof(int), key_size);
            memcpy(value, file_buffer + j + sizeof(int) + sizeof(int) + key_size, value_size);
            key[key_size] = '\0';
            value[value_size] = '\0';
            //printf("Key: %s\n", key);
            //printf("Value %s \n", value);
            //printf("Key Size: %d \n", key_size);
            //printf("Value Size %d \n", value_size);
            if (strcmp(value, tombstone) != 0){
                add_key(temp_map, key, value);
            }
            //printf("TRYING Key: %s\n", get_value(temp_map, key));
            j = j + sizeof(int) + sizeof(int) + key_size + value_size;
            //free(key);
            //free(value);
        }
    }
    //directory_buffer[current_fd_buffer_index_copy + 1] = directory_buffer[current_fd_buffer_index_copy]+1;
    //current_fd_buffer_index_copy ++;
    //printf("New Fd_buffer_index: %d \n", current_fd_buffer_index_copy);
    //lseek(dir_fd, 0L, 2);
    //int new_bytes = write(dir_fd, &directory_buffer[current_fd_buffer_index_copy],(sizeof(int)));
    //printf("new_bytes %d \n", new_bytes);
    //snprintf(path, sizeof(path), "db/%d", directory_buffer[current_fd_buffer_index]);
    //printf("Path %s \n", path);
    //fd = open(path, O_RDWR|O_CREAT, 0666);
    //memset(path, 0, 256);
    //std::unordered_map<std::string, int*> &map = master_map[directory_buffer[current_fd_buffer_index_copy]];
    //static_hash_map_array *map = construct_hash_map_array();
    //master_map[directory_buffer[current_fd_buffer_index_copy]] = map;
    //master_add_key_array(master_map, directory_buffer[current_fd_buffer_index_copy], map);
    //char new_file_buf[1024];
    // What happens if we write to directory buf at same time?? This is where we may need to rethink
    // Either need to introduce locking or a completely new buffer and merge the buffers after
    for (int i=0; i < temp_map->total_size; i++) {
        if (strcmp(temp_map->hash_map[i].key, null_value_array) == 0){
            //printf("EQUALS %d", i);
            continue;
        }
        //printf("key: %s, null_value: %s\n", temp_map->hash_map[i].key, null_value_array);
        //char *key = (char *) malloc ((pair.first.size() + 1) * sizeof(char));
        //char *value = (char *) malloc ((pair.second.size() + 1) * sizeof(char));
        //std::strcpy(key, pair.first.c_str());
        //std::strcpy(value, pair.second.c_str());
        //lseek(fd, 0L, 2);
        //printf("String key %s \n", temp_map->hash_map[i].key);
        set(temp_map->hash_map[i].key, temp_map->hash_map[i].value);
        //lseek(fd, 0L, 0);
        //int bytes_count = read(fd, new_file_buf, 1024);
        //printf("Current buffer index postwrite %d", current_fd_buffer_index_copy);
        //printf("Bytes Read after Lseek %d \n", bytes_count);
        //char* return_value = get(temp_map->hash_map[i].key, directory_buffer, current_fd_buffer_index_copy);
        //std::cout << "Return Value, " << return_value << "\n";
        /*
        if (bytes_count > PAGE_FAULT){
            directory_buffer[current_fd_buffer_index + 1] = directory_buffer[current_fd_buffer_index]+1;
            current_fd_buffer_index ++;
            //printf("New Fd_buffer_index: %d \n", current_fd_buffer_index_copy);
            int dir_byte_count = sizeof(directory_buffer);
            lseek(dir_fd, 0L, 2);
            int new_bytes = write(dir_fd, &directory_buffer[current_fd_buffer_index_copy],(sizeof(int)));
            //printf("new_bytes %d \n", new_bytes);
            char path[256];
            snprintf(path, sizeof(path), "db/%d", directory_buffer[current_fd_buffer_index_copy]);
            int fd = open(path, O_RDWR|O_CREAT, 0666);
            memset(path, 0, 256);
            //std::unordered_map<std::string, int*> &map = master_map[directory_buffer[current_fd_buffer_index_copy]];
            static_hash_map_array *map = construct_hash_map_array();
            //master_map[directory_buffer[current_fd_buffer_index_copy]] = map;
            master_add_key_array(master_map, directory_buffer[current_fd_buffer_index_copy], map);
        }
        memset(new_file_buf, 0, 1024);
        */
    }
    //lseek(fd, 0L, 2);
    int new_directory_buffer [1024];
    int j = 0;
    for (int i = original_buffer_index+1; i < current_fd_buffer_index+1;i++){
        new_directory_buffer[j] = directory_buffer[i];
        j ++;
    }
    pthread_mutex_lock(&mtx);
    //printf("entered lock\n");
    for (int i = 0; i < original_buffer_index+1; i++){
        //printf("i %d\n", i);
        char path[256];
        snprintf(path, sizeof(path), "db/%d", directory_buffer[i]);
        //printf("i %s\n", path);
        master_delete_key_array(master_map, directory_buffer[i]);
        remove(path);
        memset(path, 0, 256);
    }
    //printf("Original Buffer Index %d\n", original_buffer_index);
    //printf("new buffer index %d\n",current_fd_buffer_index_copy);
    current_fd_buffer_index = current_fd_buffer_index - (original_buffer_index+1);
    //printf("Current Fd Buffer Index %d \n", current_fd_buffer_index);
    memcpy(directory_buffer, new_directory_buffer, sizeof(int) * (current_fd_buffer_index+1));
    //printf("size of directory buffer %lu\n", sizeof(int) * (current_fd_buffer_index+1));
    int new_bytes_new = write(dir_fd, new_directory_buffer, sizeof(int) * (current_fd_buffer_index+1));
    //printf("New Bytes: %d \n", new_bytes_new);
    free_memory_hash_map(temp_map);
    //printf("exiting lock\n");
    pthread_mutex_unlock(&mtx);
    bit_flag = 0;
    return NULL;
    
}
int construct_hash_map_from_directory(){
    master_map = master_construct_hash_map_array();
    dir_fd = open("db/directory", O_RDWR|O_CREAT, 0666);
    if (dir_fd == -1){
        //printf("File Could not be opened\n");
        return -1;
    }
    int bytes_read_dir = read(dir_fd, directory_buffer, 1024);
    //printf("Bytes Read Directory %d\n", bytes_read_dir);
    int fd;
    if (bytes_read_dir == 0){
        /*
        Need to create a directory and original hash_map for files based on fd
        */
       static_hash_map_array *map = construct_hash_map_array();
       //master_map[0] = map;
       master_add_key_array(master_map, 0, map);
       directory_buffer[0] = 0;
       dir_byte_count = sizeof(int);
       //printf("size of directory %zu \n", dir_byte_count);
       int dir_bytes_read = write(dir_fd, directory_buffer,dir_byte_count);
       //printf("Bytes Written %d \n", dir_bytes_read);
       char path[256];
       snprintf(path, sizeof(path), "db/%d", directory_buffer[0]);
       fd = open(path, O_RDWR|O_CREAT, 0666);
       memset(path, 0, 256);
       current_fd_buffer_index = 0;
    }
    else{
        /*
        Construction of Keydir in memory hash map based on log files
        */
        //printf("Else Case \n");
        int file_descriptors_written = bytes_read_dir / sizeof(int);
        //printf("file descriptors Written %d \n", file_descriptors_written);
        char file_buffer[1024];
        for (int i = 0; i < file_descriptors_written; i++){
            static_hash_map_array *map = construct_hash_map_array();
            ///std::unordered_map<std::string, int*>& map = master_map[directory_buffer[i]];
            //master_map[directory_buffer[i]] = map;
            master_add_key_array(master_map, i, map);
            char path[256];
            snprintf(path, sizeof(path), "db/%d", directory_buffer[i]);
            fd = open(path, O_RDONLY, 0);
            memset(path, 0, 256);
            int bytes_read = read(fd, file_buffer, 1024);
            //printf("Bytes Read: %d\n", bytes_read);
            int key_size;
            int value_size;
            int j = 0;
            while (j < bytes_read){
                memcpy(&key_size, file_buffer + j, sizeof(int));
                memcpy(&value_size, file_buffer+ j + sizeof(int), sizeof(int));
                char * key = (char*) malloc(key_size+1);
                char * value = (char *) malloc(value_size+1);
                memcpy(key, file_buffer + j + sizeof(int) + sizeof(int), key_size);
                memcpy(value, file_buffer + j + sizeof(int) + sizeof(int) + key_size, value_size);
                key[key_size] = '\0';
                value[value_size] = '\0';
                //printf("Key: %s\n", key);
                //printf("Value %s \n", value);
                //printf("Key Size: %d \n", key_size);
                //printf("Value Size %d \n", value_size);
                //get rid of std::string
                //std::string key_s(key);
                int *arr = (int*)malloc(2 * sizeof(int));
                //printf("Pointer to array %p \n", arr);
                //printf("J: %d \n", j);
                //printf("Offset: %lu \n", sizeof(int) + sizeof(int) + key_size + value_size);
                arr[0] = j;
                arr[1] = sizeof(int) + sizeof(int) + key_size + value_size;
                //map[key_s] = arr;
                add_key_array(map, key, arr);
                j = j + sizeof(int) + sizeof(int) + key_size + value_size;
                free(key);
                free(value);
            }
            close(fd);
            current_fd_buffer_index = i;
        }
    }
    return 0;
}
    int command_line_interface(){
        int return_value = construct_hash_map_from_directory();
        if (return_value == -1){
            return -1;
        }
        char path[256];
        snprintf(path, sizeof(path), "db/%d", directory_buffer[current_fd_buffer_index]);
        int fd = open(path, O_RDWR, 0);
        memset(path, 0, 256);
        char command[3];
        char temp_buffer [1000];
        char* user_input_key;
        char * user_input_value;
        size_t length;
        while(1){
            printf("Get OR Set: ");
            scanf("%3s", command);
            if (strcmp(command, "Set") == 0){
                printf("Enter Key: ");
                scanf("%999s", temp_buffer);
                length = strlen(temp_buffer);
                user_input_key = (char *) malloc((length+1)* sizeof(char));
                strcpy(user_input_key, temp_buffer);
                memset(temp_buffer, 0, 1000);
                printf("Enter Value: ");
                scanf("%999s", temp_buffer);
                length = strlen(temp_buffer);
                user_input_value = (char*)malloc((length +1) * sizeof(char));
                strcpy(user_input_value, temp_buffer);
                memset(temp_buffer, 0, 1000);
                printf("directory buffer of current file index %d\n", directory_buffer[current_fd_buffer_index]);
                int return_error = set(user_input_key, user_input_value);
                if (return_error == -1){
                    printf("Error occured inserting Key & Value\n");
                }
                else{
                    printf("Succesfully Inserted Key: %s \n", user_input_key);

                }
                memset(command, 0, sizeof(command));
                free(user_input_key);
                free(user_input_value);
                memset(temp_buffer, 0, 1000);
            }
            else if (strcmp(command, "Get") == 0){
                printf("Enter Key: ");
                scanf("%999s", temp_buffer);
                length = strlen(temp_buffer);
                user_input_key = (char *) malloc((length+1)* sizeof(char));
                strcpy(user_input_key, temp_buffer);
                printf("Directory buffer of current file index, %d\n", directory_buffer[current_fd_buffer_index]);
                printf("Current fd index %d \n", current_fd_buffer_index);
                printf("Current size of directory buffer %lu \n", sizeof(directory_buffer) / sizeof(int));
                char * return_value = get(user_input_key);
                printf("Key: %s, -> Value: %s\n", user_input_key, return_value);
                memset(command, 0, sizeof(command));
                free(user_input_key);
                memset(temp_buffer, 0, 1000);
            }
            else if (strcmp(command, "DEL") == 0){
                printf("Enter Key: ");
                scanf("%999s", temp_buffer);
                length = strlen(temp_buffer);
                user_input_key = (char *) malloc((length+1)* sizeof(char));
                strcpy(user_input_key, temp_buffer);
                int return_error = set(user_input_key, tombstone);
                if (return_error == -1){
                    printf("Error occured Deleting Key & Value\n");
                }
                else{
                    printf("Succesfully Deleted Key: %s \n", user_input_key);
                }
                free(user_input_key);
                memset(command, 0, sizeof(command));
                memset(temp_buffer, 0, 1000);
            }
            else{
                memset(command, 0, sizeof(command));
                continue;
            }
            close(fd);
        }
        return 0;
    }
    void flush_db(){
        for (int i = 0; i < current_fd_buffer_index + 1; i++){
            char path[256];
            snprintf(path, sizeof(path), "db/%d", directory_buffer[i]);
            master_delete_key_array(master_map, directory_buffer[i]);
            /*
            auto it = master_map.find(directory_buffer[i]);
            if (it != master_map.end()) { 
                //printf("%s Path being erased\n", path);
                master_map.erase(it); 
            } 
            */
            remove(path);
        }
        // flush the directory too
        current_fd_buffer_index = 0;
        char path[] = "db/directory";
        remove(path);
    }
/*
int main(){
    int return_value = command_line_interface();
}
*/