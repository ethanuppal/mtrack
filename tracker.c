// mtrack: tracker.c
// Copyright (C) 2021 Ethan Uppal. All rights reserved.

#define MTRACK_ENABLE
#include "_tracker.h"

static malloc_trace_t trace;
#ifdef MTRACK_AUTOLOG
FILE* logfile;
#endif

void* _tmalloc(size_t n, const char* file, size_t line) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    void* block = malloc(n);
    clock_gettime(CLOCK_MONOTONIC, &end);
    if (block == NULL) {
        trace_abort("Virtual memory exhausted\n");
    }
    if (trace.allocations != NULL) {
        allocation_t a = {
            .previous = NULL,
            .pointer = block,
            .length = n,
            .state = ALLOCATION_STATE_ALLOCATED,
            .file = file,
            .line = line,
            .start = start.tv_nsec,
            .end = end.tv_nsec
        };
        #ifdef MTRACK_AUTOLOG
        dump_allocation(&a, TRACE_DUMP_MODE_LOGGING, logfile);
        #endif
        trace_append(&trace, a);
        trace.footprint += n;
    }
    return block;
}

void* _trealloc(void* ptr, size_t n, const char* file, size_t line) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    void* block = realloc(ptr, n);
    clock_gettime(CLOCK_MONOTONIC, &end);
    if (block == NULL) {
        trace_abort("Virtual memory exhausted\n");
    }
    if (trace.allocations != NULL) {
        allocation_t a = {
            .previous = ptr,
            .pointer = block,
            .length = n,
            .state = ALLOCATION_STATE_REALLOCATED,
            .file = file,
            .line = line,
            .start = start.tv_nsec,
            .end = end.tv_nsec
        };
        #ifdef MTRACK_AUTOLOG
        dump_allocation(&a, TRACE_DUMP_MODE_LOGGING, logfile);
        #endif
        trace_append(&trace, a);
        trace.footprint += n;
    }
    return block;
}

void _tfree(void* ptr, const char* file, size_t line) {
    allocation_t a = {
        .previous = ptr,
        .pointer = NULL,
        .length = 0,
        .state = ALLOCATION_STATE_FREED,
        .file = file,
        .line = line,
    };
    #ifdef MTRACK_AUTOLOG
    dump_allocation(&a, TRACE_DUMP_MODE_LOGGING, logfile);
    fflush(logfile);
    #endif

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    free(ptr);
    clock_gettime(CLOCK_MONOTONIC, &end);
    if (trace.allocations != NULL) {
        const allocation_t* allocation = trace_search(&trace, ptr);
        const size_t length = allocation == NULL ? 0 : allocation->length;
        trace.footprint -= length;
        a.start = start.tv_nsec;
        a.end = end.tv_nsec;
        trace_append(&trace, a);
    }
}

static void trace_append(malloc_trace_t* trace, allocation_t allocation) {
    if (trace->length + 1 > trace->capacity) {
        trace->capacity *= 2;
        trace->allocations = (allocation_t*)realloc(trace->allocations,
                                                    sizeof(allocation_t)
                                                    * trace->capacity);
        if (trace->allocations == NULL) {
            trace_abort("Unable to continue tracing as virtual memory is exhausted\n");
        }
    }
    trace->allocations[trace->length++] = allocation;
}

static allocation_t* trace_search(malloc_trace_t* trace, void* ptr) {
    allocation_t* allocation = trace->allocations;
    for (size_t i = 0; i < trace->length; i++) {
        if (allocation->pointer == ptr) {
            return allocation;
        }
        allocation++;
    }
    return NULL;
}

void tinit() {
    trace.length = 0;
    trace.capacity = 4;
    trace.allocations = (allocation_t*)malloc(sizeof(allocation_t)
                                                     * trace.capacity);
    trace.footprint = 0;

    if (trace.allocations == NULL) {
        trace_abort("Unable to setup tracing as virtual memory is exhausted\n");
    }

    #ifdef MTRACK_AUTOLOG
    // Empty log file
    fclose(fopen("mtrack.log", "w"));
    logfile = fopen("mtrack.log", "a");
    #endif
}

void tdestroy() {
    free(trace.allocations);
    trace.allocations = NULL;
}

static void dump_allocation(const allocation_t* allocation,
                             trace_dump_mode_t dump_mode, FILE* stream)
{
    switch (dump_mode) {
        case TRACE_DUMP_MODE_READABLE: {
            switch (allocation->state) {
                case ALLOCATION_STATE_ALLOCATED: {
                    fprintf(stream, "%08zx bytes allocated (%p)",
                            allocation->length,
                            allocation->pointer);
                    break;
                }
                case ALLOCATION_STATE_REALLOCATED: {
                    fprintf(stream, "%p reallocated to %08zx bytes (%p)",
                            allocation->previous,
                            allocation->length,
                            allocation->pointer);
                    break;
                }
                case ALLOCATION_STATE_FREED: {
                    fprintf(stream, "%zu bytes freed (%p)",
                            allocation->length,
                            allocation->previous);
                    break;
                }
            }
            fprintf(stream, " in %ldμs\n",
                    (allocation->end - allocation->start) / 1000);
        }
        case TRACE_DUMP_MODE_LOGGING: {
            switch (allocation->state) {
                case ALLOCATION_STATE_ALLOCATED: {
                    fprintf(stream, "+ %zu %zu %zu %s\n",
                    (uintptr_t)allocation->pointer,
                    allocation->length,
                    allocation->line,
                    allocation->file);
                    break;
                }
                case ALLOCATION_STATE_REALLOCATED: {
                    fprintf(stream, "- %zu %zu %s\n",
                            (uintptr_t)allocation->previous,
                            allocation->line,
                            allocation->file);
                    fprintf(stream, "+ %zu %zu %zu %s\n",
                            (uintptr_t)allocation->pointer,
                            allocation->length,
                            allocation->line,
                            allocation->file);
                    break;
                }
                case ALLOCATION_STATE_FREED: {
                    fprintf(stream, "- %zu %zu %s\n",
                            (uintptr_t)allocation->previous,
                            allocation->line,
                            allocation->file);
                    break;
                }
            }
        }
    }
}

void tdump(trace_dump_mode_t dump_mode) {
    FILE* stream = stderr;
    if (dump_mode == TRACE_DUMP_MODE_LOGGING) {
        // https://stackoverflow.com/questions/4815251/how-do-i-clear-the-whole-contents-of-a-file-in-c
        fclose(fopen("mtrack.log", "w"));
        stream = fopen("mtrack.log", "a");
    }
    const allocation_t* allocation = trace.allocations;
    const size_t old_length = trace.length;
    for (size_t i = 0; i < old_length; i++) {
        dump_allocation(allocation, dump_mode, stream);
        allocation++;
    }
    if (dump_mode == TRACE_DUMP_MODE_LOGGING) {
        fclose(stream);
    }
}

void _tdump(void) {
    tdump(TRACE_DUMP_MODE_LOGGING);
}

void tdump_on_exit() {
    atexit(_tdump);
}

size_t tusage() {
    return trace.footprint;
}
