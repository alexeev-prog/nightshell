// utils.h
#ifndef UTILS_H
#define UTILS_H

char** split_by_delims(const char* line, const char* delims, int* count);
char** split_into_tokens(const char* line, int* count);
void free_tokens(char** tokens, int count);

#endif
