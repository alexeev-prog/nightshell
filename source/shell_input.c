/**
 * @file shell_input.c
 * @brief Implementation of advanced shell input system
 */
#include "shell_input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/types.h>
#include "_default.h"

// Default color scheme
static const ShellColors kDefaultColors = {
    .command = ANSI_GREEN,
    .error = ANSI_RED,
    .argument = ANSI_CYAN,
    .prompt = ANSI_BLUE ANSI_BOLD,
    .suggestion = ANSI_YELLOW,
    .reset = ANSI_RESET
};

// Default prompt configuration
static const ShellPrompt kDefaultPrompt = {
    .format = DEFAULT_PROMPT_FORMAT,
    .user_color = ANSI_GREEN,
    .dir_color = ANSI_CYAN,
    .symbol_color = ANSI_BLUE ANSI_BOLD,
    .symbol = NULL,  // Auto-detect based on UID
    .dynamic_dir = true
};

// Current configuration
static ShellConfig current_config;

// Terminal state preservation
static struct termios original_termios;

// Command registry for autocompletion
static char** command_registry = NULL;
static size_t command_count = 0;
static size_t registry_capacity = 0;

// User and host info cache
static char cached_username[MAX_USER_LENGTH] = "";
static char cached_hostname[MAX_HOST_LENGTH] = "";

/**
 * @brief Get username with caching
 */
static const char* get_username(void) {
    if (cached_username[0] == '\0') {
        const struct passwd* pw = getpwuid(getuid());
        if (pw) {
            strncpy(cached_username, pw->pw_name, sizeof(cached_username) - 1);
        } else {
            strcpy(cached_username, "unknown");
        }
    }
    return cached_username;
}

/**
 * @brief Get prompt symbol
 */
static const char* get_prompt_symbol(void) {
    if (current_config.prompt.symbol) {
        return current_config.prompt.symbol;
    }
    return (getuid() == 0) ? "#" : "$";
}

/**
 * @brief Calculate visible length of string (ignoring escape sequences)
 */
static size_t visible_length(const char* str) {
    size_t len = 0;
    int in_escape = 0;

    for (const char* p = str; *p; p++) {
        if (*p == '\033') {
            in_escape = 1;
            continue;
        }

        if (in_escape) {
            if (*p == 'm') in_escape = 0;
            continue;
        }

        len++;
    }

    return len;
}

/**
 * @brief Initialize terminal for raw input
 */
static void enable_raw_mode(void) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &original_termios);
    raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/**
 * @brief Restore terminal to original state
 */
static void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

/**
 * @brief Initialize shell input system
 */
void shell_input_init(const ShellConfig* config) {
    // Apply configuration
    if (config) {
        current_config = *config;
    } else {
        current_config.colors = kDefaultColors;
        current_config.prompt = kDefaultPrompt;
        current_config.max_suggestions = MAX_SUGGESTIONS;
    }

    // Initialize command registry
    registry_capacity = INITIAL_REG_CAPACITY;
    command_registry = malloc(registry_capacity * sizeof(char*));
    if (!command_registry) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    command_count = 0;

    // Register default commands
    register_command("help");
    register_command("version");
    register_command("exit");
    register_command("clear");
}

/**
 * @brief Cleanup shell input resources
 */
void shell_input_cleanup(void) {
    for (size_t i = 0; i < command_count; i++) {
        free(command_registry[i]);
    }
    free(command_registry);
    command_registry = NULL;
    command_count = 0;
    registry_capacity = 0;
}

/**
 * @brief Register command for autocompletion
 */
void register_command(const char* command) {
    if (command_count >= registry_capacity) {
        const size_t new_capacity = registry_capacity * 2;
        char** new_reg = realloc(command_registry, new_capacity * sizeof(char*));
        if (!new_reg) {
            perror("realloc");
            return;
        }
        command_registry = new_reg;
        registry_capacity = new_capacity;
    }

    command_registry[command_count] = strdup(command);
    if (!command_registry[command_count]) {
        perror("strdup");
        return;
    }
    command_count++;
}

/**
 * @brief Generate prompt string
 */
static const char* generate_prompt(void) {
    static char prompt_buffer[MAX_PROMPT_LENGTH];
    char cwd[MAX_PROMPT_LENGTH];
    const char* format = current_config.prompt.format ? current_config.prompt.format : DEFAULT_PROMPT_FORMAT;

    if (current_config.prompt.dynamic_dir) {
        if (!getcwd(cwd, sizeof(cwd))) {
            strcpy(cwd, "?");
        }
    } else {
        static char static_cwd[MAX_PROMPT_LENGTH] = "";
        if (static_cwd[0] == '\0') {
            if (!getcwd(static_cwd, sizeof(static_cwd))) {
                strcpy(static_cwd, "?");
            }
        }
        strcpy(cwd, static_cwd);
    }

    const char* username = get_username();
    const char* symbol = get_prompt_symbol();

    char* dest = prompt_buffer;
    const char* src = format;
    size_t remaining = sizeof(prompt_buffer) - 1;

    while (*src && remaining > 0) {
        if (*src == '%') {
            src++;
            const char* color_code = "";
            const char* value = "";

            switch (*src) {
                case 'u': // Username
                    color_code = current_config.prompt.user_color;
                    value = username;
                    break;
                case 'd': // Directory (last part)
                    color_code = current_config.prompt.dir_color;
                    value = strrchr(cwd, '/');
                    value = value ? value + 1 : cwd;
                    break;
                case 'w': // Full working directory
                    color_code = current_config.prompt.dir_color;
                    value = cwd;
                    break;
                case 's': // Prompt symbol
                    color_code = current_config.prompt.symbol_color;
                    value = symbol;
                    break;
                case '%': // Literal %
                    value = "%";
                    break;
                default: // Unknown specifier
                    src--;
                    value = "%";
                    break;
            }

            // Add color if specified
            if (color_code && *color_code) {
                size_t len = strlen(color_code);
                if (len < remaining) {
                    memcpy(dest, color_code, len);
                    dest += len;
                    remaining -= len;
                }
            }

            // Add value
            size_t val_len = strlen(value);
            if (val_len < remaining) {
                memcpy(dest, value, val_len);
                dest += val_len;
                remaining -= val_len;
            }

            // Add reset if color was added
            if (color_code && *color_code && remaining > strlen(current_config.colors.reset)) {
                const char* reset = current_config.colors.reset;
                size_t reset_len = strlen(reset);
                memcpy(dest, reset, reset_len);
                dest += reset_len;
                remaining -= reset_len;
            }

            if (*src) src++;
        } else {
            *dest++ = *src++;
            remaining--;
        }
    }

    *dest = '\0';
    return prompt_buffer;
}

/**
 * @brief Display suggestions
 */
static void show_suggestions(const char* prefix, size_t prefix_len) {
    printf("\n");
    size_t shown_count = 0;

    for (size_t i = 0; i < command_count && shown_count < current_config.max_suggestions; i++) {
        if (strncmp(command_registry[i], prefix, prefix_len) == 0) {
            printf("%s%s%s\n",
                   current_config.colors.suggestion,
                   command_registry[i],
                   current_config.colors.reset);
            shown_count++;
        }
    }

    if (shown_count == 0) {
        printf("%sNo suggestions%s\n", current_config.colors.error, current_config.colors.reset);
    }

    fflush(stdout);
}

/**
 * @brief Get command color based on validity
 */
static const char* get_command_color(const char* command) {
    for (size_t i = 0; i < command_count; i++) {
        if (strcmp(command_registry[i], command) == 0) {
            return current_config.colors.command;
        }
    }
    return current_config.colors.error;
}

/**
 * @brief Redraw input line with syntax highlighting
 */
static void redraw_input(const char* input, size_t cursor_pos) {
    // Generate and print prompt
    const char* prompt = generate_prompt();
    printf("%s%s%s", CLEAR_LINE, CURSOR_HOME, prompt);

    // Calculate visible prompt length
    const size_t prompt_len = visible_length(prompt);

    // Print input with syntax highlighting
    if (input[0] != '\0') {
        // Tokenize input for highlighting
        char buffer[MAX_INPUT_LENGTH];
        strncpy(buffer, input, sizeof(buffer));
        buffer[sizeof(buffer)-1] = '\0';

        char* token = strtok(buffer, " ");
        if (token) {
            // Highlight command
            printf("%s%s%s", get_command_color(token), token, current_config.colors.reset);

            // Highlight arguments
            while ((token = strtok(NULL, " "))) {
                printf(" %s%s%s", current_config.colors.argument, token, current_config.colors.reset);
            }
        }
    }

    // Position cursor correctly
    const size_t total_pos = prompt_len + cursor_pos;
    printf("\033[%zuG", total_pos + 1); // +1 because terminal columns are 1-based
    fflush(stdout);
}

/**
 * @brief Handle escape sequence input
 */
static void handle_escape_sequence(size_t* cursor_pos, size_t input_len) {
    char seq[2];
    if (read(STDIN_FILENO, &seq[0], 1) <= 0) return;
    if (read(STDIN_FILENO, &seq[1], 1) <= 0) return;

    if (seq[0] == '[') {
        switch (seq[1]) {
            case 'C':  // Right arrow
                if (*cursor_pos < input_len) (*cursor_pos)++;
                break;
            case 'D':  // Left arrow
                if (*cursor_pos > 0) (*cursor_pos)--;
                break;
            case 'A':  // Up arrow - History
                // Placeholder for history implementation
                break;
            case 'B':  // Down arrow - History
                // Placeholder for history implementation
                break;
            case 'Z':  // Shift+Tab
                // Handle reverse tab if needed
                break;
            default:
                break;
        }
    }
}

/**
 * @brief Handle tab completion
 */
static void handle_tab_completion(const char* input, size_t cursor_pos,
                                 size_t input_len, size_t* cursor_pos_ptr) {
    // Find current word
    size_t word_start = cursor_pos;
    while (word_start > 0 && !isspace(input[word_start-1])) {
        word_start--;
    }

    const size_t word_len = cursor_pos - word_start;
    if (word_len == 0) return;

    char prefix[MAX_CMD_NAME_LENGTH];
    const size_t copy_len = word_len < sizeof(prefix) ? word_len : sizeof(prefix) - 1;
    strncpy(prefix, input + word_start, copy_len);
    prefix[copy_len] = '\0';

    show_suggestions(prefix, copy_len);
    redraw_input(input, *cursor_pos_ptr);
}

/**
 * @brief Handle backspace
 */
static void handle_backspace(char* input, size_t* input_len, size_t* cursor_pos) {
    if (*cursor_pos > 0) {
        memmove(input + *cursor_pos - 1, input + *cursor_pos, *input_len - *cursor_pos + 1);
        (*cursor_pos)--;
        (*input_len)--;
    }
}

/**
 * @brief Handle printable character input
 */
static void handle_printable_char(char* input, size_t* input_len, size_t* cursor_pos, char ch) {
    if (*input_len < MAX_INPUT_LENGTH - 1) {
        memmove(input + *cursor_pos + 1, input + *cursor_pos, *input_len - *cursor_pos + 1);
        input[*cursor_pos] = ch;
        (*cursor_pos)++;
        (*input_len)++;
    }
}

/**
 * @brief Read a line from user with advanced features
 */
char* shell_readline(void) {
    enable_raw_mode();

    char input[MAX_INPUT_LENGTH] = {0};
    size_t input_len = 0;
    size_t cursor_pos = 0;

    // Initial draw
    redraw_input(input, cursor_pos);

    while (true) {
        char ch;
        const ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);
        if (bytes_read <= 0) continue;

        if (ch == '\t') {  // Tab completion
            handle_tab_completion(input, cursor_pos, input_len, &cursor_pos);
        } else if (ch == 127 || ch == '\b') {  // Backspace
            handle_backspace(input, &input_len, &cursor_pos);
            redraw_input(input, cursor_pos);
        } else if (ch == '\n') {  // Enter
            printf("\n");
            break;
        } else if (ch == 27) {  // Escape sequence
            handle_escape_sequence(&cursor_pos, input_len);
            redraw_input(input, cursor_pos);
        } else if (isprint(ch)) {  // Printable characters
            handle_printable_char(input, &input_len, &cursor_pos, ch);
            redraw_input(input, cursor_pos);
        }
    }

    disable_raw_mode();
    return strdup(input);
}
