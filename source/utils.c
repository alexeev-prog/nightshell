// utils.c
#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 64
#define MAX_TOKEN_LEN 256

// Разбивает строку по разделителям (&&, ||, ;)
char** split_by_delims(const char* line, const char* delims, int* count) {
    char** tokens = malloc(MAX_TOKENS * sizeof(char*));
    *count = 0;
    const char* start = line;
    const char* current = line;
    int in_quote = 0;
    char quote_char = 0;

    while (*current && *count < MAX_TOKENS - 1) {
        // Обработка кавычек
        if (*current == '"' || *current == '\'') {
            if (in_quote && *current == quote_char) {
                in_quote = 0;
            } else if (!in_quote) {
                in_quote = 1;
                quote_char = *current;
            }
        }

        // Поиск разделителей вне кавычек
        if (!in_quote) {
            if (strchr(delims, *current)) {
                // Найден разделитель
                int len = current - start;
                if (len > 0) {
                    tokens[*count] = strndup(start, len);
                    (*count)++;
                }

                tokens[*count] = strndup(current, 1);
                (*count)++;

                start = current + 1;
            }
        }
        current++;
    }

    // Остаток строки
    if (start < current) {
        tokens[*count] = strdup(start);
        (*count)++;
    }

    return tokens;
}

// Разбивает строку на токены с учетом кавычек
char** split_into_tokens(const char* line, int* count) {
    char** tokens = malloc(MAX_TOKENS * sizeof(char*));
    *count = 0;
    char token[MAX_TOKEN_LEN];
    int pos = 0;
    int in_quote = 0;
    char quote_char = 0;

    while (*line && *count < MAX_TOKENS - 1) {
        if (isspace(*line) && !in_quote) {
            if (pos > 0) {
                token[pos] = '\0';
                tokens[*count] = strdup(token);
                (*count)++;
                pos = 0;
            }
        } else if ((*line == '"' || *line == '\'') && (!in_quote || *line == quote_char)) {
            if (in_quote && *line == quote_char) {
                in_quote = 0;
            } else {
                in_quote = 1;
                quote_char = *line;
            }
        } else {
            token[pos++] = *line;
        }
        line++;
    }

    if (pos > 0) {
        token[pos] = '\0';
        tokens[*count] = strdup(token);
        (*count)++;
    }

    tokens[*count] = NULL;
    return tokens;
}

void free_tokens(char** tokens, int count) {
    for (int i = 0; i < count; i++) {
        free(tokens[i]);
    }
    free(tokens);
}
