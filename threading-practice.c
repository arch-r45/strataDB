#include <pthread.h>
#include <stdio.h>
#include <string.h> 
/*
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
 void *(*start)(void *), void *arg);
*/
static int glob = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void* function1(void *arg){
    int n = *(int *)arg;
    for (int i = 0; i < n; i++){
        pthread_mutex_lock(&mtx);
        glob += 1;
        pthread_mutex_unlock(&mtx);
        continue;
    }
    printf("Function 1: %d\n", n);
    return NULL;
}
void* function2(void *arg){
    int n = *(int *)arg;
    for (int i = 0; i < n; i++){
        pthread_mutex_lock(&mtx);
        glob += 1;
        pthread_mutex_unlock(&mtx);
        continue;
    }
    printf("Function 2: %d\n", n);
    return NULL;
}
int main(){
    pthread_t t1;
    pthread_t t2;
    void *res;
    void *res2;
    int s;
    int t;
    int arg1 = 100000000;
    int arg2 = 100; 
    s = pthread_create(&t1, NULL, function1, &arg1);
    s = pthread_create(&t2, NULL, function2, &arg2);
    printf("Message from main()\n");
    int return1 = pthread_join(t1, NULL);
    int return2 = pthread_join(t2, NULL);
    printf("Glob: %d\n", glob);
    printf("Threads returned \n");
}