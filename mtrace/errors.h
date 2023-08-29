// malloc-tracker: errors.h
// Copyright (C) 2023 Ethan Uppal. All rights reserved.

#pragma once

enum domain {
    ERROR
};

void message(enum domain, const char* msg, const char* fix);
