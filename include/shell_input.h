/**
 * @file shell_input.h
 * @brief Advanced input system for shell with syntax highlighting and autocompletion
 */
#ifndef SHELL_INPUT_H
#define SHELL_INPUT_H

#include <stdbool.h>
#include <stddef.h>

// Constants for buffer sizes
#define MAX_INPUT_LENGTH 1024
#define MAX_PROMPT_LENGTH 256
#define MAX_CMD_NAME_LENGTH 128
#define MAX_SUGGESTIONS 5
#define INITIAL_REG_CAPACITY 32
#define MAX_HOST_LENGTH 64
#define MAX_USER_LENGTH 32

#define MAX_SUGGESTIONS 5
#define MAX_USER_LENGTH 32
#define MAX_HOST_LENGTH 64
#define MAX_PROMPT_LENGTH 256
#define MAX_INPUT_LENGTH 1024
#define MAX_HISTORY 100
#define HISTORY_FILE ".nightshell_history"

// Terminal control sequences
#define CLEAR_LINE "\033[2K"
#define CURSOR_HOME "\r"
#define CURSOR_FORWARD(n) "\033[" #n "C"

// Default prompt format
#define DEFAULT_PROMPT_FORMAT "[%u] %w %s "

/**
 * @brief Color codes for syntax highlighting
 */
typedef struct {
    const char* command;    ///< Color for valid commands
    const char* error;    ///< Color for invalid commands/errors
    const char* argument;    ///< Color for arguments
    const char* prompt;    ///< Color for prompt
    const char* suggestion;    ///< Color for autocomplete suggestions
    const char* reset;    ///< Reset color code
} ShellColors;

/**
 * @brief Shell prompt configuration
 */
typedef struct {
    const char* format;    ///< Prompt format string
    const char* user_color;    ///< Color for username
    const char* dir_color;    ///< Color for directory
    const char* symbol_color;    ///< Color for prompt symbol
    const char* symbol;    ///< Prompt symbol (NULL for auto)
    bool dynamic_dir;    ///< Enable dynamic directory in prompt
} ShellPrompt;

/**
 * @brief Shell input configuration
 */
typedef struct {
    ShellColors colors;    ///< Color scheme configuration
    ShellPrompt prompt;    ///< Prompt configuration
    size_t max_suggestions;    ///< Max autocomplete suggestions to show
} ShellConfig;

/**
 * @brief Initialize shell input system
 *
 * @param config Configuration for input system
 */
void shell_input_init(const ShellConfig* config);

/**
 * @brief Cleanup shell input resources
 */
void shell_input_cleanup(void);

/**
 * @brief Read a line from user with advanced features
 *
 * @return char* User input (must be freed by caller)
 */
char* shell_readline(void);

/**
 * @brief Register command for autocompletion
 *
 * @param command Command name to register
 */
void register_command(const char* command);

#endif    // SHELL_INPUT_H
