#ifndef LIST_H
#define LIST_H

#include <pthread.h>
#include <stddef.h>

typedef struct ListNode {
    void *data;
    struct ListNode *next;
} ListNode;

typedef struct {
    ListNode *head;
    size_t size;
    pthread_mutex_t mutex;
} List;

List *list_create();
void list_destroy(List *list, void (*free_fn)(void *));
void list_append(List *list, void *data);
void *list_pop_front(List *list);
void list_remove(List *list, void *data, int (*cmp)(void*,void*));
void list_foreach(List *list, void (*fn)(void*, void*), void *arg);
size_t list_size(List *list);

#endif




// #ifndef LIST_H
// #define LIST_H

// #include <pthread.h>
// #include <stddef.h>

// typedef struct ListNode {
//     void *data;
//     struct ListNode *next;
// } ListNode;

// typedef struct {
//     ListNode *head;
//     size_t size;
//     pthread_mutex_t mutex;
// } List;

// List *list_create();
// void list_destroy(List *list, void (*free_fn)(void *));
// void list_append(List *list, void *data);
// void *list_pop_front(List *list);
// void list_remove(List *list, void *data, int (*cmp)(void*,void*));
// void list_foreach(List *list, void (*fn)(void*, void*), void *arg);
// size_t list_size(List *list);

// #endif