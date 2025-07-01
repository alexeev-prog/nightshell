#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include "cmdparser.h"
#include "executor.h"
#include "shell_input.h"
#include "utils.h"
#include "tasks_processing.h"
#include "_default.h"

#define VERSION "v0.1.0"
#define null nullptr

extern tasks tasks_structure;

/**
 * @brief Signal handler for SIGINT during command execution
 */
static void handle_exec_sigint(int sig) {
    // Simply interrupt the current foreground process
    // Actual termination will be handled in tasks_processing
}

int process_commands(char* input) {
    int num_tokens;
    char** tokens = split_by_delims(input, "&|;", &num_tokens);
    int status = 0;
    int last_status = 0;
    char* next_operator = NULL;

    for (int i = 0; i < num_tokens; i++) {
        if (strlen(tokens[i]) == 0) {
            continue;
        }

        // Обработка операторов
        if (strcmp(tokens[i], "&") == 0) {
            next_operator = "&";
            continue;
        } else if (strcmp(tokens[i], "|") == 0) {
            next_operator = "|";
            continue;
        } else if (strcmp(tokens[i], ";") == 0) {
            next_operator = NULL;
            continue;
        }

        int arg_count;
        char** args = split_into_tokens(tokens[i], &arg_count);

        if (!next_operator || (next_operator && strcmp(next_operator, "&") == 0 && last_status == 0)
            || (next_operator && strcmp(next_operator, "|") == 0 && last_status != 0))
        {
            // Set SIGINT handler for command execution
            void (*original_sigint)(int) = signal(SIGINT, handle_exec_sigint);
            status = execute(args);
            signal(SIGINT, original_sigint);
            last_status = status;
        }

        free_tokens(args, arg_count);
        next_operator = NULL;
    }

    free_tokens(tokens, num_tokens);
    return status;
}

int main(int argc, char** argv) {
    setlocale(LC_ALL, "C.UTF-8");

    int help_flag = 0;
    int version_flag = 0;
    int status = 1;

    struct CommandOption options[4] = {
        {"Help info", "help", 'h', 0, NULL, &help_flag},
        {"Version", "version", 'v', 0, NULL, &version_flag},
    };

    struct CLIMetadata meta = {.prog_name = argv[0],
                               .description = "",
                               .usage_args = "[ARGUMENTS...]",
                               .options = options,
                               .options_count = sizeof(options) / sizeof(options[0])};

    ShellConfig config = {.colors = {.command = ANSI_GREEN,
                                     .error = ANSI_RED,
                                     .argument = ANSI_CYAN,
                                     .prompt = ANSI_BLUE,
                                     .suggestion = ANSI_YELLOW,
                                     .reset = ANSI_RESET},
                          .prompt = {.format = ANSI_MAGENTA "[%u" ANSI_RESET "@" ANSI_CYAN "%w" ANSI_RESET
                                                            "]" ANSI_GREEN "%s " ANSI_RESET,
                                     .user_color = ANSI_MAGENTA,
                                     .dir_color = ANSI_CYAN,
                                     .symbol_color = ANSI_GREEN,
                                     .symbol = NULL,
                                     .dynamic_dir = true},
                          .max_suggestions = 5};

    int pos_index = parse_options(argc, argv, meta.options, meta.options_count);

    if (pos_index < 0) {
        return 1;
    }

    // Help flag
    if (help_flag) {
        print_help(&meta);
        return 0;
    }

    if (version_flag) {
        printf("Version: %s\n", VERSION);
        return 0;
    }

    shell_input_init(&config);

    signal(SIGINT, kill_foreground_task);
	signal(SIGCHLD, mark_ended_task);

    do {
        char* input = shell_readline();
        if (!input) {
            break;
        }
        status = process_commands(input);
        free(input);
    } while (status == 1);

    shell_input_cleanup();

    return 0;
}
