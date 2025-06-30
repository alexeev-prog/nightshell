#include "cmdparser.h"
#include "shell_input.h"
#include <stdio.h>
#include "_default.h"

#define VERSION "v0.1.0"
#define null nullptr

int main(int argc, char **argv) {
    int help_flag = 0;
    int version_flag = 0;

    struct CommandOption options[4] = {
        {"Help info", "help",    'h', 0, null, &help_flag},     // Help flag
        {"Version", "version",    'v', 0, null, &version_flag},     // Help flag
    };

    struct CLIMetadata meta = {
        .prog_name = argv[0],
        .description = "",
        .usage_args = "[ARGUMENTS...]",
        .options = options,
        .options_count = sizeof(options) / sizeof(options[0])
    };

    ShellConfig config = {
        .colors = {
            .command = ANSI_GREEN,
            .error = ANSI_RED,
            .argument = ANSI_CYAN,
            .prompt = ANSI_BLUE,
            .suggestion = ANSI_YELLOW,
            .reset = ANSI_RESET
        },
        .prompt = {
            .format = ANSI_MAGENTA "[%u" ANSI_RESET "@"
                         ANSI_CYAN "%w" ANSI_RESET "]"
                         ANSI_GREEN "%s " ANSI_RESET,
            .user_color = ANSI_MAGENTA,
            .dir_color = ANSI_CYAN,
            .symbol_color = ANSI_GREEN,
            .symbol = NULL, // Auto-detect ($ or #)
            .dynamic_dir = true
        },
        .max_suggestions = 5
    };

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

    while (true) {
        char* input = shell_readline();
        if (!input) break;

        printf("You entered: %s\n", input);

        free(input);
    }

    shell_input_cleanup();

    return 0;
}
