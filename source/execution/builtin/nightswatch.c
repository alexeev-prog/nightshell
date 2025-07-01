#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "colors.h"

/**
 * @brief      Read a specific line from a file
 *
 * @param      filename     The filename
 * @param[in]  line_number  The line number (1-indexed)
 *
 * @return     Allocated string containing the line (without newline),
 *             or NULL on error
 */
char* r_line(const char* filename, int line_number) {
    if (line_number < 1) {
        return NULL;
    }

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return NULL;
    }

    int c;
    int current_line = 1;
    size_t len = 0;
    char* line = NULL;

    // Skip lines until we reach the desired line
    while (current_line < line_number) {
        c = fgetc(fp);
        if (c == EOF) {
            fclose(fp);
            return NULL;
        }
        if (c == '\n') {
            current_line++;
        }
    }

    // Read the desired line
    while ((c = fgetc(fp)) != EOF && c != '\n') {
        char* temp = realloc(line, len + 2);    // +2 for new char and null terminator
        if (temp == NULL) {
            free(line);
            fclose(fp);
            return NULL;
        }
        line = temp;
        line[len++] = (char)c;
    }

    if (line != NULL) {
        line[len] = '\0';
    } else if (c != EOF) {    // Handle empty line case
        line = malloc(1);
        if (line != NULL) {
            line[0] = '\0';
        }
    }

    fclose(fp);
    return line;
}

/**
 * @brief      Get newborn from /proc/loadavg
 *
 * @return     status code
 */
int nightwatch_newborn(char** args) {
    char* line = r_line("/proc/loadavg", 1);    // Read first line

    if (line == NULL) {
        print_message("Loading file error /proc/loadavg", ERROR);
        return 1;
    }

    char* token = strtok(line, " ");
    char* children = NULL;

    // Extract last token (newborn PID)
    while (token != NULL) {
        children = token;
        token = strtok(NULL, " ");
    }

    if (children != NULL) {
        printf("Children born: %s\n", children);
    } else {
        print_message("No token found in /proc/loadavg", ERROR);
    }

    free(line);    // Free allocated memory
    return 1;
}
