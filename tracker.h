// malloc-tracker: tracker.h
// Copyright (C) 2021 Ethan Uppal. All rights reserved.

#pragma once

#define _POSIX_C_SOURCE 0x200809L
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    TRACE_DUMP_MODE_READABLE,
    TRACE_DUMP_MODE_LOGGING
} trace_dump_mode_t;

#ifdef MTRACK_ENABLE

void* _tmalloc(size_t n, const char* file, size_t line);
void* _trealloc(void* ptr, size_t n, const char* file, size_t line);
void _tfree(void* ptr, const char* file, size_t line);

#define tmalloc(n) _tmalloc(n, __FILE__, __LINE__);
#define trealloc(ptr, n) _trealloc(ptr, n, __FILE__, __LINE__);
#define tfree(ptr) _tfree(ptr, __FILE__, __LINE__);

#else

#define tmalloc malloc
#define trealloc realloc
#define tfree free

#endif

void tinit(void);
void tdestroy(void);
void tdump(trace_dump_mode_t dump_mode);
size_t tusage(void);
