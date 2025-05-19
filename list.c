#include "list.h"
#include <stdlib.h>

List *list_create() {
    List *list = malloc(sizeof(List));
    if (!list) return NULL;
    list->head = NULL;
    list->size = 0;
    pthread_mutex_init(&list->mutex, NULL);
    return list;
}

void list_destroy(List *list, void (*free_fn)(void *)) {
    if (!list) return;
    pthread_mutex_lock(&list->mutex);
    ListNode *cur = list->head;
    while (cur) {
        ListNode *tmp = cur;
        cur = cur->next;
        if (free_fn) free_fn(tmp->data);
        free(tmp);
    }
    pthread_mutex_unlock(&list->mutex);
    pthread_mutex_destroy(&list->mutex);
    free(list);
}

void list_append(List *list, void *data) {
    if (!list) return;
    ListNode *node = malloc(sizeof(ListNode));
    node->data = data;
    node->next = NULL;

    pthread_mutex_lock(&list->mutex);
    if (!list->head) {
        list->head = node;
    } else {
        ListNode *cur = list->head;
        while (cur->next) cur = cur->next;
        cur->next = node;
    }
    list->size++;
    pthread_mutex_unlock(&list->mutex);
}

void *list_pop_front(List *list) {
    if (!list) return NULL;
    pthread_mutex_lock(&list->mutex);
    if (!list->head) {
        pthread_mutex_unlock(&list->mutex);
        return NULL;
    }
    ListNode *node = list->head;
    void *data = node->data;
    list->head = node->next;
    free(node);
    list->size--;
    pthread_mutex_unlock(&list->mutex);
    return data;
}

void list_remove(List *list, void *data, int (*cmp)(void*,void*)) {
    if (!list) return;
    pthread_mutex_lock(&list->mutex);
    ListNode **cur = &list->head;
    while (*cur) {
        if (cmp((*cur)->data, data) == 0) {
            ListNode *tmp = *cur;
            *cur = tmp->next;
            free(tmp);
            list->size--;
            break;
        }
        cur = &(*cur)->next;
    }
    pthread_mutex_unlock(&list->mutex);
}

void list_foreach(List *list, void (*fn)(void*, void*), void *arg) {
    if (!list || !fn) return;
    pthread_mutex_lock(&list->mutex);
    for (ListNode *cur = list->head; cur; cur = cur->next) {
        fn(cur->data, arg);
    }
    pthread_mutex_unlock(&list->mutex);
}

size_t list_size(List *list) {
    if (!list) return 0;
    pthread_mutex_lock(&list->mutex);
    size_t sz = list->size;
    pthread_mutex_unlock(&list->mutex);
    return sz;
}

// #include "list.h"
// #include <unistd.h>
// #include <stdlib.h>

// List *list_create() {
//     List *list = malloc(sizeof(List));
//     list->head = NULL;
//     list->size = 0;
//     pthread_mutex_init(&list->mutex, NULL);
//     return list;
// }

// void list_destroy(List *list, void (*free_fn)(void *)) {
//     pthread_mutex_lock(&list->mutex);
//     ListNode *cur = list->head;
//     while (cur) {
//         ListNode *tmp = cur;
//         cur = cur->next;
//         if (free_fn) free_fn(tmp->data);
//         free(tmp);
//     }
//     pthread_mutex_unlock(&list->mutex);
//     pthread_mutex_destroy(&list->mutex);
//     free(list);
// }

// void list_append(List *list, void *data) {
//     ListNode *node = malloc(sizeof(ListNode));
//     node->data = data;
//     node->next = NULL;
//     pthread_mutex_lock(&list->mutex);
//     if (!list->head) {
//         list->head = node;
//     } else {
//         ListNode *cur = list->head;
//         while (cur->next) cur = cur->next;
//         cur->next = node;
//     }
//     list->size++;
//     pthread_mutex_unlock(&list->mutex);
// }

// void *list_pop_front(List *list) {
//     pthread_mutex_lock(&list->mutex);
//     if (!list->head) {
//         pthread_mutex_unlock(&list->mutex);
//         return NULL;
//     }
//     ListNode *node = list->head;
//     list->head = node->next;
//     void *data = node->data;
//     free(node);
//     list->size--;
//     pthread_mutex_unlock(&list->mutex);
//     return data;
// }

// void list_remove(List *list, void *data, int (*cmp)(void*,void*)) {
//     pthread_mutex_lock(&list->mutex);
//     ListNode *cur = list->head, *prev = NULL;
//     while (cur) {
//         if (cmp(cur->data, data) == 0) {
//             if (prev) prev->next = cur->next;
//             else list->head = cur->next;
//             free(cur);
//             list->size--;
//             break;
//         }
//         prev = cur;
//         cur = cur->next;
//     }
//     pthread_mutex_unlock(&list->mutex);
// }

// void list_foreach(List *list, void (*fn)(void*, void*), void *arg) {
//     pthread_mutex_lock(&list->mutex);
//     for (ListNode *cur = list->head; cur; cur = cur->next) fn(cur->data, arg);
//     pthread_mutex_unlock(&list->mutex);
// }

// size_t list_size(List *list) {
//     pthread_mutex_lock(&list->mutex);
//     size_t s = list->size;
//     pthread_mutex_unlock(&list->mutex);
//     return s;
// }