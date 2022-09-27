// malloc-tracker: main.c: A testing program for malloc-tracker.
// Copyright (C) 2021 Ethan Uppal. All rights reserved.

#define MTRACK_ENABLE
#include "tracker.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct _list_t {
    int value;
    struct _list_t* next;
} list_t;

void print_free(list_t* list) {
    printf("node: %d\n", list->value);
    if (list->next != NULL) {
        print_free(list->next);
    }
    tfree(list);
}

int main(void) {
    tinit();

    list_t* head = (list_t*)tmalloc(sizeof(list_t));
    head->value = 0;
    list_t* list = head;
    for (int i = 1; i < 10; i++) {
        list->next = (list_t*)tmalloc(sizeof(list_t));
        list = list->next;
        list->value = (int)i;
    }
    list->next = NULL;

    printf("Before freeing, the program has allocated %zu byte(s)\n", tusage());

    void* LOL = tmalloc(893); // random leak
    printf("fake ptr usage: %p\n", LOL);

    print_free(head);
    print_free(head); // double free example

    // Only if not using MTRACK_AUTOLOG
    // tdump(TRACE_DUMP_MODE_LOGGING);

    tdestroy();
    return 0;
}
