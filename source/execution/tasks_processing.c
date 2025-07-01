/**
 * @title SheGang Linux Shell | Tasks Processing (POSIX)
 * @filename tasks_processing.c
 * @brief Module with tasks management and background/foreground tasks structures
 * @authors Alexeev Bronislav
 * @license MIT License
 **/
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#include "colors.h"
#include "executor.h"
#include "slogger.h"

// Functions definitions
int kill(pid_t pid, int);

extern int global_status_code;

/**
 * Struct background task
 */
struct background_task_t {
    pid_t pid;
    int is_finished;
    char* timestamp;
    char* command;
};
typedef struct background_task_t bg_task;

/**
 * Struct foreground task
 */
struct foreground_task_t {
    pid_t pid;
    int is_finished;
};
typedef struct foreground_task_t fg_task;

/**
 * Struct for all tasks
 */
struct tasks_t {
    fg_task foreground_task;
    bg_task* background_task;
    size_t cursor;
    size_t capacity;
};
typedef struct tasks_t tasks;

/**
 * tasks_structure
 */
tasks tasks_structure = {
    .foreground_task = {.pid = -1, .is_finished = 1}, .background_task = 0, .cursor = 0, .capacity = 0};

/**
 * @brief Set foreground task
 */
void set_foreground_task(pid_t pid) {
    tasks_structure.foreground_task.pid = pid;
    tasks_structure.foreground_task.is_finished = 0;
}

/**
 * @brief Add new background task
 */
int add_background_task(pid_t pid, char* name) {
    bg_task* bt;

    if (tasks_structure.cursor >= tasks_structure.capacity) {
        tasks_structure.capacity = tasks_structure.capacity * 2 + 1;
        tasks_structure.background_task =
            (bg_task*)realloc(tasks_structure.background_task, sizeof(bg_task) * tasks_structure.capacity);

        if (tasks_structure.background_task == 0 || tasks_structure.background_task == NULL) {
            print_message("Couldn't reallocate buffer for background tasks!", ERROR);
            return -1;
        }
    }

    printf("[%zu] task started\n", tasks_structure.cursor);

    bt = &tasks_structure.background_task[tasks_structure.cursor];

    bt->pid = pid;
    bt->is_finished = 0;

    time_t timestamp = time(NULL);
    bt->timestamp = ctime(&timestamp);

    bt->command = name;

    tasks_structure.cursor += 1;

    return 1;
}

/**
 * @brief Kill foreground task
 */
void kill_foreground_task(void) {
    if (tasks_structure.foreground_task.pid != -1) {
        kill(tasks_structure.foreground_task.pid, SIGTERM);
        tasks_structure.foreground_task.is_finished = 1;
        printf("\n");
    }
}

/**
 * @brief Terminate background task
 */
int term_background_task(char** args) {
    char* idx_str;
    int proc_idx = 0;

    if (args[1] == NULL) {
        print_message("No process index to stop", ERROR);
    } else {
        idx_str = args[1];

        while (*idx_str >= '0' && *idx_str <= '9') {
            proc_idx = (proc_idx * 10) + ((*idx_str) - '0');
            idx_str += 1;
        }

        if (*idx_str != '\0' || (size_t)proc_idx >= tasks_structure.cursor) {
            print_message("Incorrect background process index!", ERROR);
        } else if (tasks_structure.background_task[proc_idx].is_finished == 0) {
            kill(tasks_structure.background_task[proc_idx].pid, SIGTERM);
        }
    }

    return 1;
}

/**
 * @brief Check if task is background
 */
int is_background_task(char** args) {
    int last_arg_id = 0;

    while (args[last_arg_id + 1] != NULL) {
        last_arg_id++;
    }

    if (strcmp(args[last_arg_id], "&") == 0) {
        args[last_arg_id] = NULL;
        return 1;
    }

    return 0;
}

/**
 * @brief Create and launch task
 */
int launch_task(char** args) {
    pid_t pid;
    int background = is_background_task(args);

    pid = fork();

    if (pid < 0) {
        log_error("Couldn't create child process (pid: %d)\n", (int)pid);
    } else if (pid == 0) {
        // Child process
        signal(SIGINT, SIG_DFL);  // Reset to default handler

        if (execvp(args[0], args) != 0) {
            log_error("Couldn't execute unknown command: %s", args[0]);
        }

        exit(-1);
    } else {
        if (background == 1) {
            if (add_background_task(pid, args[0]) == -1) {
                quit_from_shell(args);
            }
        } else {
            set_foreground_task(pid);

            int child_status;
            if (waitpid(pid, &child_status, 0) == -1) {
                if (errno != EINTR) {
                    print_message("Couldn't track the completion of the process", WARNING);
                    global_status_code = -1;
                }
            }

            // Check if child was terminated by signal
            if (WIFSIGNALED(child_status)) {
                int term_sig = WTERMSIG(child_status);
                if (term_sig == SIGINT) {
                    printf("\n");
                }
            }
        }
    }

    return 1;
}

/**
 * @brief Mark ended tasks
 */
void mark_ended_task(void) {
    bg_task* bt;

    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (pid == tasks_structure.foreground_task.pid) {
            tasks_structure.foreground_task.is_finished = 1;
        } else {
            for (size_t i = 0; i < tasks_structure.cursor; i++) {
                bt = &tasks_structure.background_task[i];

                if (bt->pid == pid) {
                    printf("Task %zu is finished\n", i);
                    bt->is_finished = 1;
                    break;
                }
            }
        }
    }
}
