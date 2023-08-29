# mtrack: A malloc debugger

Memory leaks are some of the most common programming errors and some of the easiest to create. To debug these issues, mtrack and mtrace analyze your memory usage and locate bugs, provided you replace `malloc`, `realloc` and `free` with the variants that have a `t` prefix.

## Overview

Simply add the following lines to the top of your program:

```c
#define MTRACK_ENABLE
#include "tracker.h"
```

You can just as easily undefine `MTRACK_ENABLE` to disable tracking functionality: `tmalloc`, `trealloc` and `tfree` will simply be expanded to their standard library variants.

It is highly recommended you define `MTRACK_AUTOLOG` in your compiler arguments because this will ensure that logs are generated even if the program crashes. If you do not set this flag, you will have to call `tdump(TRACE_DUMP_MODE_LOGGING)` manually at some point before calling `tdestroy`.

> NOTE: I am developing another method that uses `atexit`, which will be highly preferable to expensive I/O for every dynamic memory operation.

## Usage

To view the current memory footprint of a program, use `tusage`. To manually log the allocations, reallocations and frees in a format meant to be easily digestive by parsers to the standard error use `tdump`. Otherwise, use `MTRACK_AUTOLOG` as explained above (RECOMMENDED).

To initialize tracing, call `tinit` at the beginning of your program. To end tracing, call `tdestroy` (Consider registering an `atexit`). Note that any use of the tracing functions after `tdestroy` is undefined behavior, although I attempted to mitigate this in my design: the `tmalloc`, `trealloc`, and `tfree` functions will simply act as the standard library variants do, but `tdump` and `tusage` will produce a crash or unexpected results.

This repository contains an example program `main.c` to showcase these features. The program contains multiple memory errors, but we will use our tools to debug it.

Firstly, let us run the program to see what occurs.

```
Before freeing, the program has allocated 160 byte(s)
fake ptr usage: 0x7f048f305280
node: 0
node: 1
node: 2
node: 3
node: 4
node: 5
node: 6
node: 7
node: 8
node: 9
node: 0
node: 57849384
node: 0
main(43243,0x1e3d96600) malloc: *** error for object 0x6000056e0006: pointer being freed was not allocated
main(43243,0x1e3d96600) malloc: *** set a breakpoint in malloc_error_break to debug
Abort trap: 6
```

Ouch! Not to fear, however, for we used the `t`-prefix variants and setup logging. That means the program has generated a `mtrack.log` file in the directory.

```
+ 125553155801088 16 25 main.c
+ 125553155801104 16 29 main.c
+ 125553155801120 16 29 main.c
+ 125553155801136 16 29 main.c
+ 125553155801152 16 29 main.c
+ 125553155801168 16 29 main.c
+ 125553155801184 16 29 main.c
+ 125553155801200 16 29 main.c
+ 125553155801216 16 29 main.c
+ 125553155801232 16 29 main.c
+ 140555145200256 893 37 main.c
- 125553155801232 19 main.c
- 125553155801216 19 main.c
- 125553155801200 19 main.c
- 125553155801184 19 main.c
- 125553155801168 19 main.c
- 125553155801152 19 main.c
- 125553155801136 19 main.c
- 125553155801120 19 main.c
- 125553155801104 19 main.c
- 125553155801088 19 main.c
- 125553155784707 19 main.c
```

If you don't understand what any of this means, that's ok. You don't need to. Thankfully, `mtrace` does. If we run `mtrace` in the same directory as the `mtrack.log` file, we get the message "2 issues found." and a new file: `mtrace.analysis`.

```
bad free: Pointer 0x61f5026be003 freed at main.c:19 was never allocated
leak: Pointer 0x7f5c858ec280 (893 bytes) last allocated at main.c:37 was not freed.
```

That makes much more sense! We see two issues: a bad free and a memory leak. Let's look at the memory leak first. We have leaked 893 bytes on line 37. That line looks like:

```c
void* LOL = tmalloc(893); // random leak
```

Now we know that we need to `tfree` the `LOL` pointer after it has been used.

Moving on to the bad free, we see that at line 19 a pointer is freed that was never allocated.

```c
void print_free(list_t* list) {
    printf("node: %d\n", list->value);
    if (list->next != NULL) {
        print_free(list->next);
    }
    tfree(list); // <-- line 19
}
```

It looks like that is getting a corrupted pointer. The only place that pointer could come from is the argument to `print_free`. It looks like the logic inside `print_free` is fine, provided `list->next` is always a valid pointer or `NULL`. That means we will have to look outside the function at when it is called.

```c
print_free(head);
print_free(head); // double free example
```

It turns out that the function is called twice, leading to already freed pointers being passed into the free function. Removing the extra call gets rid of the bad free issue. Now, `mtrace` tells us: "No issues found." 🎉
