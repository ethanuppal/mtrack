// mtrace: errors.c
// Copyright (C) 2023 Ethan Uppal. All rights reserved.

#include <stdio.h>
#include "errors.h"

const char* domain_strings[] = {
    [ERROR] = "error"
};

void message(enum domain domain, const char* msg, const char* fix) {
    printf("%s: %s.", domain_strings[domain], msg);
    if (fix) {
        printf(" %s.", fix);
    }
    putchar('\n');
}
