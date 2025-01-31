/*
 * Copyright (c) 2024
 */

#ifndef FOPERATION_H
#define FOPERATION_H

#include "main.h"
#include "common.h"

void BuildConfig(char*);

/** Confirm the file exists or Not.
 * path:   file path.
 * return: 0: exist, 1: not exists.*/
int8_t AddFile(const char*);

/*
 * Overwrite the file exists or Not.
 * path:   file path.
 * return: 0: exist, 1: not exists.
*/
int8_t OverWriteFile(const char*);

#endif /* FOPERATION_H */
