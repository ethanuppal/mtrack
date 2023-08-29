// malloc-tracker: allocations.c
// Copyright (C) 2021 Ethan Uppal. All rights reserved.

#include "allocations.h"
#include <stdlib.h> // exit, malloc
#include <string.h> // memset
#include <limits.h> // PATH_MAX
#include <stdint.h> // uint64_t, uint32_t, uintptr_t
#include "errors.h" // message
#include "../_tracker.h"

char path[PATH_MAX + 1];

void mtrack_allocations_init(mtrack_allocations_t* allocations) {
    allocations->length = 0;
    allocations->capacity = 4;
    allocations->array = (mtrack_instance_t*)malloc(sizeof(mtrack_instance_t)
                                                    * allocations->capacity);
    if (allocations->array == NULL) {
        trace_abort("Unable to setup tracing because virtual memory is exhausted\n");
    }
}

void mtrack_allocations_destroy(mtrack_allocations_t* allocations) {
    // for (size_t i = 0; i < allocations->length; i++) {
    //     free((char*)allocations->array[i].file);
    // }
    free(allocations->array);
}

mtrack_instance_t* mtrack_allocations_get(mtrack_allocations_t* allocations,
                                          void* pointer) {
    // First try to find it
    for (size_t i = 0; i < allocations->length; i++) {
        if (allocations->array[i].pointer == pointer) {
            return &allocations->array[i];
        }
    }
    // If not make new one
    if (allocations->length + 1 > allocations->capacity) {
        allocations->capacity *= 2;
        allocations->array = realloc(allocations->array,
                                     sizeof(mtrack_instance_t)
                                     * allocations->capacity);
    }
    // Setup new one
    mtrack_instance_t* instance = &allocations->array[allocations->length++];
    instance->pointer = pointer;
    instance->bytes = 0;
    instance->start_line = 0;
    instance->start_file = NULL;
    instance->end_line = 0;
    instance->end_file = NULL;
    instance->freed = true;
    return instance;
}

int mtrack_allocations_alloc(mtrack_allocations_t* allocations, void* pointer,
                             size_t bytes, size_t line, const char* file,
                             FILE* ostream) {
    mtrack_instance_t* instance = mtrack_allocations_get(allocations, pointer);
    if (!instance->freed) {
        fprintf(ostream, "corruption: Pointer %p (%zu bytes) previously allocated at %s:%zu was reallocated at %s:%zu with %zu bytes.\n", pointer, instance->bytes, instance->start_file, instance->start_line, file, line, bytes);
        return MTRACK_ISSUE_DETECTED;
    }
    instance->bytes = bytes;
    instance->start_line = line;
    instance->start_file = file;
    instance->freed = false;
    return 0;
}

int mtrack_allocations_free(mtrack_allocations_t* allocations, void* pointer,
                            size_t line, const char* file, FILE* ostream) {
    mtrack_instance_t* instance = mtrack_allocations_get(allocations, pointer);
    if (instance->freed) {
        if (instance->end_file) {
            fprintf(ostream, "double free: Pointer %p previously freed at %s:%zu was freed again at %s:%zu\n", pointer, instance->end_file, instance->end_line, file, line);
        } else {
            fprintf(ostream, "bad free: Pointer %p freed at %s:%zu was never allocated\n", pointer, file, line);
        }
        return MTRACK_ISSUE_DETECTED;
    }
    instance->end_line = line;
    instance->end_file = file;
    instance->freed = true;
    return 0;
}

int mtrack_parse(mtrack_allocations_t* allocations, char* line,
                 size_t length, FILE* ostream) {
    if (length == 0) {
        // Skip empty line
        return 0;
    }
    if (length < 3) {
        message(ERROR, "Invalid log", "A line in the log is too short");
        exit(EXIT_FAILURE);
    }

    const char operation = line[0];
    line += 2;

    uintptr_t pointer_int;

    memset(path, 0, PATH_MAX + 1);
    size_t line_n;
    char* file = malloc(128); // it's fine to leak memory I guess? ironic

    switch (operation) {
        case '+': {
            size_t bytes;
            sscanf(line, "%zu %zu %zu %127[^\n]", &pointer_int, &bytes, &line_n, file);
            void* pointer = (void*)pointer_int;
            if (mtrack_allocations_alloc(allocations, pointer, bytes, line_n,
                                         file, ostream)
                == MTRACK_ISSUE_DETECTED) {
                return MTRACK_ISSUE_DETECTED;
            }
            return 0;
        }
        case '-': {
            sscanf(line, "%zu %zu %127[^\n]", &pointer_int, &line_n, file);
            void* pointer = (void*)pointer_int;
            if (mtrack_allocations_free(allocations, pointer, line_n, file,
                                        ostream) == MTRACK_ISSUE_DETECTED) {
                return MTRACK_ISSUE_DETECTED;
            }
            return 0;
        }
        default: {
            printf("'%c' %s\n", operation, line);
            message(ERROR, "Invalid log", "Unknown log entry prefix character");
            exit(EXIT_FAILURE);
        }
    }
}

int mtrack_scan(mtrack_allocations_t* allocations, FILE* ostream) {
    int ret = 0;
    for (size_t i = 0; i < allocations->length; i++) {
        mtrack_instance_t* instance = &allocations->array[i];
        if (!instance->freed) {
            fprintf(ostream, "leak: Pointer %p (%zu bytes) last allocated at %s:%zu was not freed.\n", instance->pointer, instance->bytes, instance->start_file, instance->start_line);
            ret++;
        }
    }
    return ret;
}
