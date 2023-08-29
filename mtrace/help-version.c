// malloc-tracker: help-version.c
// Copyright (C) 2021 Ethan Uppal. All rights reserved.

#include "help-version.h"
#include <stdio.h>

const char mtrack_help_text[] =
    "Usage: %s [OPTION]...\n"
    "\n"
    "A tool to parse and analyze mtrace logs.\n"
    "\n"
    "Options:\n"
    "  -i FILE      Provides the location of the input log. Default: mtrack.log.\n"
    "  -o FILE      Provides the location of the resulting analysis. Default: mtrack.analysis.\n"
    "  --help       Shows this help.\n"
    "  --version    Shows version and license information.\n";

inline void mtrack_show_help(const char* program_name) {
    printf(mtrack_help_text, program_name);
}

const char mtrack_version_text[] =
    "mtrack 1.0\n"
    "Copyright (C) 2021 Ethan Uppal. All rights reserved.\n";

inline void mtrack_show_version() {
    fputs(mtrack_version_text, stdout);
}
