// malloc-tracker: mtrace.c: Analyzes a tracking log for its properties.
// Copyright (C) 2021 Ethan Uppal. All rights reserved.

#include <string.h> // strncmp
#include <stdlib.h> // exit
#include "help-version.h"
#include "allocations.h" // MTRACK_ISSUE_DETECTED
#include "../_tracker.h"
#include "errors.h" // message

// Returns true if the given strings are equal in length.
#define strequ(str, str2) ((str) == NULL ? 0 : strcmp(str, str2) == 0)

static void parse_args(int argc, const char* argv[], const char** infile,
                       const char** outfile) {
    if (strequ(argv[1], "--help")) {
        mtrack_show_help(argv[0]);
        exit(EXIT_SUCCESS);
    } else if (strequ(argv[1], "--version")) {
        mtrack_show_version();
        exit(EXIT_SUCCESS);
    }
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'i': {
                    i++;
                    *infile = argv[i];
                    if (*infile == NULL) {
                        message(ERROR, "Expected file name after -i option",
                                NULL);
                        exit(EXIT_FAILURE);
                    }
                }
                case 'o': {
                    i++;
                    *outfile = argv[i];
                    if (*outfile == NULL) {
                        message(ERROR, "Expected file name after -o option",
                                NULL);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}

int main(int argc, char const* argv[]) {
    const char* infile = "mtrack.log";
    const char* outfile = "mtrace.analysis";
    parse_args(argc, argv, &infile, &outfile);

    mtrack_allocations_t allocations;
    mtrack_allocations_init(&allocations);
    char* line = NULL;
    size_t n = 0;
    FILE* istream = fopen(infile, "r");
    if (istream == NULL) {
        message(ERROR, "Could not find log file in directory", "Run --help for a list of options");
        return EXIT_FAILURE;
    }
    FILE* ostream = fopen(outfile, "wa");
    if (ostream == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }
    size_t issue_count = 0;
    while (getline(&line, &n, istream) != -1) {
        if (mtrack_parse(&allocations, line, n, ostream)
            == MTRACK_ISSUE_DETECTED) {
            issue_count++;
        }
        free(line);
        line = NULL;
        n = 0;
    }
    int leaks = mtrack_scan(&allocations, ostream);
    if (leaks > 0) {
        issue_count += leaks;
    }
    mtrack_allocations_destroy(&allocations);
    fclose(istream);
    fclose(ostream);
    if (issue_count > 0) {
        printf("%zu issue%s found.\n", issue_count,
               issue_count == 1 ? "" : "s");
    } else {
        printf("No issues found.\n");
    }
    return 0;
}
