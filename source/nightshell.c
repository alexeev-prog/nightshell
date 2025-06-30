#include "cmdparser.h"
#include <stdio.h>

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
    }

    return 0;
}
