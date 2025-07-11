/**
 * @title SheGang Linux Shell | Colors and formatting
 * @filename colors.c
 * @brief Module with colors, print functions, and shell ps1
 * @authors Alexeev Bronislav
 * @license MIT License
 **/
#include "colors.h"

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#include "_default.h"

extern char* username_color;
extern char* pwd_color;
extern char* curr_time_color;

int global_status_code = 0;

/**
 * @brief      Gets the color by name.
 *
 * @param[in]  color_name  The color name
 *
 * @return     The color by name.
 */
char* get_color_by_name(const char* color_name) {
    struct {
        char *text, *code;
    } colors[] = {{.text = "RESET", .code = RESET},
                  {.text = "BLACK", .code = BLACK},
                  {.text = "RED", .code = RED},
                  {.text = "GREEN", .code = GREEN},
                  {.text = "YELLOW", .code = YELLOW},
                  {.text = "BLUE", .code = BLUE},
                  {.text = "MAGENTA", .code = MAGENTA},
                  {.text = "CYAN", .code = CYAN},
                  {.text = "WHITE", .code = WHITE},
                  {.text = "GRAY", .code = GRAY}};

    const int len = sizeof(colors) / sizeof(colors[0]);

    for (int i = 0; i < len; ++i) {
        if (strcmp(color_name, colors[i].text) == 0) {
            return colors[i].code;
        }
    }

    return NULL;
}

/**
 * @brief      Simple function for print message with new line
 *
 * @param[in]  message  The message
 * @param      const  char* message Message for printing
 */
void println(const char* message) {
    printf("%s\n", message);
}

/**
 * @brief      Print colored message with new line
 *
 * @param[in]  message        The message
 * @param      message_color  The message color
 * @param      const  char* message Message for printing
 * @param      char*  message_color ANSI Code Color
 */
void println_colored(const char* message, char* message_color) {
    printf("%s%s%s\n", message_color, message, RESET);
}

/**
 * @brief      Print colored message without new line
 *
 * @param[in]  message        The message
 * @param      message_color  The message color
 * @param      const  char* message Message for printing
 * @param      char*  message_color ANSI Code Color
 */
void print_colored(const char* message, char* message_color) {
    printf("%s%s%s", message_color, message, RESET);
}

/**
 * @brief      function for print beautiful colored message
 *
 * @param[in]  message       The message
 * @param[in]  message_type  The message type
 * @param      const  char* message Message text
 * @param      int    message_type Color
 *
 * @return     void
 */
void print_message(const char* message, int message_type) {
    const char* color;
    const char* format;
    const char* msgtype_string;

    switch (message_type) {
        case DEBUG:
            color = CYAN;
            format = BOLD;
            msgtype_string = "[DEBUG]";
            break;
        case INFO:
            color = GREEN;
            format = BOLD;
            msgtype_string = "[INFO]";
            break;
        case WARNING:
            color = YELLOW;
            format = DIM;
            msgtype_string = "[WARNING]";
            break;
        case ERROR:
            color = RED;
            format = BOLD;
            msgtype_string = "[ERROR]";
            break;
        default:
            color = WHITE;
            format = RESET;
            msgtype_string = "[DEFAULT]";
            break;
    }

    if (message_type == ERROR) {
        fprintf(stderr, "%s%s%s%s%s %s\n", RESET, color, format, msgtype_string, RESET, message);
    } else {
        printf("%s%s%s%s %s%s\n", RESET, color, format, msgtype_string, RESET, message);
    }

    printf(RESET);
}
