//
// Created by dorian on 09/09/2026.
//

#ifndef SERIALISE
#define SERIALISE
#include "parse.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int serialise_line(struct line* line, FILE* fp);
void translate(const char* source, const char* out);
#endif
