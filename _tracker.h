// mtrack: _tracker.h
// Copyright (C) 2021 Ethan Uppal. All rights reserved.

#pragma once

#include "tracker.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

typedef enum {
    ALLOCATION_STATE_ALLOCATED,
    ALLOCATION_STATE_REALLOCATED,
    ALLOCATION_STATE_FREED
} allocation_state_t;

typedef struct {
    void* previous;
    void* pointer;
    size_t length;
    allocation_state_t state;
    const char* file;
    size_t line;
    long start;
    long end;
} allocation_t;

typedef struct {
    size_t length;
    size_t capacity;
    allocation_t* allocations;
    size_t footprint;
} malloc_trace_t;

static void trace_append(malloc_trace_t* trace, allocation_t allocation);
static allocation_t* trace_search(malloc_trace_t* trace, void* ptr);
static void dump_allocation(const allocation_t* allocation,
                             trace_dump_mode_t dump_mode, FILE* stream);

#define trace_abort(fmt, ...) do { \
    fprintf(stderr, "malloc-trace abort:%s:%d: " fmt, __FILE__, __LINE__, ## __VA_ARGS__); \
    exit(EXIT_FAILURE); \
} while (0)
#define trace_warning(fmt, ...) do { \
    fprintf(stderr, "malloc-trace warning:%s:%d: " fmt, __FILE__, __LINE__, ## __VA_ARGS__); \
} while (0)
