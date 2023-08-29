// mtrace: allocations.h
// Copyright (C) 2021 Ethan Uppal. All rights reserved.

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#define MTRACK_ISSUE_DETECTED 1

typedef struct {
    void* pointer;
    size_t bytes;
    size_t start_line;
    const char* start_file;
    size_t end_line;
    const char* end_file;
    bool freed;
} mtrack_instance_t;

typedef struct {
    size_t length;
    size_t capacity;
    mtrack_instance_t* array;
} mtrack_allocations_t;

void mtrack_allocations_init(mtrack_allocations_t* allocations);
void mtrack_allocations_destroy(mtrack_allocations_t* allocations);

int mtrack_parse(mtrack_allocations_t* allocations, char* line,
                 size_t length, FILE* ostream);
int mtrack_scan(mtrack_allocations_t* allocations, FILE* ostream);
