// malloc-tracker: errors.c
// Copyright (C) 2023 Ethan Uppal. All rights reserved.

#include <stdio.h>
#include "errors.h"

void message(enum domain, const char* msg, const char* fix) {
    printf("error: %s.", msg);
    if (fix) {
        printf(" %s.", fix);
    }
    putchar('\n');
}
