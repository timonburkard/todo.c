#ifndef TODO_H
#define TODO_H

#include <stdint.h>

/**
 * @brief Mark code as unreachable
 *
 */
#define UNREACHABLE()                                                               \
    do {                                                                            \
        fprintf(stderr, "UNREACHABLE code reached at %s:%d\n", __FILE__, __LINE__); \
        abort();                                                                    \
    } while (0)

/**
 * @brief Mark code as unreachable with annotation
 *
 */
#define UNREACHABLE_A(text)                                                                    \
    do {                                                                                       \
        fprintf(stderr, "UNREACHABLE code reached at %s:%d | %s\n", __FILE__, __LINE__, text); \
        abort();                                                                               \
    } while (0)

typedef enum {
    TODO_ERROR_OK       = 0,
    TODO_ERROR_GENERAL  = 1,
    TODO_ERROR_ARGUMENT = 2,
    TODO_ERROR_SQL      = 3,
} todo_error_t;

/**
 * @brief Add a new TODO
 *
 * For full description see `cli:help_add()`
 *
 * @param argc -- Number of arguments from the user AFTER the `todo add` command
 * @param argv -- Arguments from the user AFTER the `todo add` command

 * @return todo_error_t -- Error code
 */
todo_error_t todo_add(int argc, char** argv);

/**
 * @brief List TODOs
 *
 * For full description see `cli:help_list()`
 *
 * @param argc -- Number of arguments from the user AFTER the `todo list` command
 * @param argv -- Arguments from the user AFTER the `todo list` command

 * @return todo_error_t -- Error code
 */
todo_error_t todo_list(int argc, char** argv);

/**
 * @brief Set or change the due date of a TODO
 *
 * For full description see `cli:help_due()`
 *
 * @param argc -- Number of arguments from the user AFTER the `todo due` command
 * @param argv -- Arguments from the user AFTER the `todo due` command

 * @return todo_error_t -- Error code
 */
todo_error_t todo_due(uint32_t id, char* due_date);

/**
 * @brief Mark a TODO as done
 *
 * For full description see `cli:help_done()`
 *
 * @param argc -- Number of arguments from the user AFTER the `todo done` command
 * @param argv -- Arguments from the user AFTER the `todo done` command

 * @return todo_error_t -- Error code
 */
todo_error_t todo_done(uint32_t id);

/**
 * @brief Interactively review the open TODOs to decide what to do with them
 *
 * For full description see `cli:help_review()`
 *
 * @param argc -- Number of arguments from the user AFTER the `todo review` command
 * @param argv -- Arguments from the user AFTER the `todo review` command

 * @return todo_error_t -- Error code
 */
todo_error_t todo_review(void);

#endif // TODO_H
