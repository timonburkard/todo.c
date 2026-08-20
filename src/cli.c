#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "todo.h"
#include "version.h"

static void help(bool is_error);
static void help_add(bool is_error);
static void help_list(bool is_error);
static void help_due(bool is_error);
static void help_done(bool is_error);
static void help_review(bool is_error);

static void version(void);

static todo_error_t add(int argc, char** argv);
static todo_error_t list(int argc, char** argv);
static todo_error_t due(int argc, char** argv);
static todo_error_t done(int argc, char** argv);
static todo_error_t review(int argc, char** argv);

typedef struct {
    const char* name;
    todo_error_t (*function)(int, char**);
} cmd_t;

static const cmd_t commands[] = {
    {.name = "add",    .function = add   },
    {.name = "list",   .function = list  },
    {.name = "due",    .function = due   },
    {.name = "done",   .function = done  },
    {.name = "review", .function = review},
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        help(true);
        return TODO_ERROR_ARGUMENT;
    }

    if ((strcmp(argv[1], "--version") == 0) || (strcmp(argv[1], "-V") == 0)) {
        version();
        return TODO_ERROR_OK;
    }

    if ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "help") == 0)) {
        help(false);
        return TODO_ERROR_OK;
    }

    for (uint8_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if (strcmp(argv[1], commands[i].name) == 0) {
            return commands[i].function(argc - 2, &argv[2]);
        }
    }

    help(true);
    return TODO_ERROR_ARGUMENT;
}

static void version(void)
{
    printf("v" TODO_VERSION "\n");
}

#define HELP_ON_SPECIFIC_CMD(argc, argv) (((argc) > 0) && ((strcmp((argv)[0], "--help") == 0) || (strcmp((argv)[0], "-h") == 0)))

/**
 * @brief Handle ``add`` command
 *
 * @param argc -- Number of arguments from the user AFTER the `todo add` command
 * @param argv -- Arguments from the user AFTER the `todo add` command
 *
 * @return todo_error_t -- error code
 */
static todo_error_t add(int argc, char** argv)
{
    todo_error_t error = TODO_ERROR_OK;

    if (HELP_ON_SPECIFIC_CMD(argc, argv)) {
        help_add(false);
        return TODO_ERROR_OK;
    }

    if (argc < 3) {
        help_add(true);
        return TODO_ERROR_ARGUMENT;
    }

    error = todo_add(argc, argv);
    if (error == TODO_ERROR_ARGUMENT) {
        help_add(true);
    }

    return error;
}

/**
 * @brief Handle `list` command
 *
 * @param argc -- Number of arguments from the user AFTER the `todo list` command
 * @param argv -- Arguments from the user AFTER the `todo list` command
 *
 * @return todo_error_t -- error code
 */
static todo_error_t list(int argc, char** argv)
{
    todo_error_t error = TODO_ERROR_OK;

    if (HELP_ON_SPECIFIC_CMD(argc, argv)) {
        help_list(false);
        return TODO_ERROR_OK;
    }

    error = todo_list(argc, argv);
    if (error == TODO_ERROR_ARGUMENT) {
        help_list(true);
    }

    return error;
}

/**
 * @brief Handle `due` command
 *
 * @param argc -- Number of arguments from the user AFTER the `todo due` command
 * @param argv -- Arguments from the user AFTER the `todo due` command
 *
 * @return todo_error_t -- error code
 */
static todo_error_t due(int argc, char** argv)
{
    todo_error_t error = TODO_ERROR_OK;

    if (HELP_ON_SPECIFIC_CMD(argc, argv)) {
        help_due(false);
        return TODO_ERROR_OK;
    }

    if (argc != 2) {
        help_due(true);
        return TODO_ERROR_ARGUMENT;
    }

    int32_t id = atoi(argv[0]);
    if (id <= 0) {
        help_due(true);
        return TODO_ERROR_ARGUMENT;
    }

    error = todo_due((uint32_t)id, argv[1]);
    if (error == TODO_ERROR_ARGUMENT) {
        help_due(true);
    }

    return error;
}

/**
 * @brief Handle `done` command
 *
 * @param argc -- Number of arguments from the user AFTER the `todo done` command
 * @param argv -- Arguments from the user AFTER the `todo done` command
 *
 * @return todo_error_t -- error code
 */
static todo_error_t done(int argc, char** argv)
{
    todo_error_t error = TODO_ERROR_OK;

    if (HELP_ON_SPECIFIC_CMD(argc, argv)) {
        help_done(false);
        return TODO_ERROR_OK;
    }

    if (argc != 1) {
        help_done(true);
        return TODO_ERROR_ARGUMENT;
    }

    int32_t id = atoi(argv[0]);
    if (id <= 0) {
        help_done(true);
        return TODO_ERROR_ARGUMENT;
    }

    error = todo_done((uint32_t)id);
    if (error == TODO_ERROR_ARGUMENT) {
        help_done(true);
    }

    return error;
}

/**
 * @brief Handle `review` command
 *
 * @param argc -- Number of arguments from the user AFTER the `todo review` command
 * @param argv -- Arguments from the user AFTER the `todo review` command
 *
 * @return todo_error_t -- error code
 */
static todo_error_t review(int argc, char** argv)
{
    todo_error_t error = TODO_ERROR_OK;

    if (HELP_ON_SPECIFIC_CMD(argc, argv)) {
        help_review(false);
        return TODO_ERROR_OK;
    }

    if (argc != 0) {
        help_review(true);
        return TODO_ERROR_ARGUMENT;
    }

    error = todo_review();
    if (error == TODO_ERROR_ARGUMENT) {
        help_review(true);
    }

    return error;
}

#define ERROR_HOW_TO_USE "ERROR, this is how to use:\n\n"

/**
 * @brief Print general help message
 *
 * @param is_error -- true: User did a wrong input, so we print an error message and the help message
 *                 -- false: User did nothing wrong, he just requested to print the help message
 */
static void help(bool is_error)
{
    if (is_error) {
        printf(ERROR_HOW_TO_USE);
    }

    printf("A small C command-line TODO app backed by SQLite\n"
           "\n"
           "\x1b[4mUsage:\x1b[24m todo <COMMAND>\n"
           "\n"
           "\x1b[4mCommands:\x1b[24m\n"
           "  add     Add a new TODO\n"
           "  list    List TODOs\n"
           "  due     Set or change a TODO's due date\n"
           "  done    Mark a TODO as done\n"
           "  review  Interactively review open TODOs\n"
           "  help    Print this message or the help of the given subcommand(s)\n"
           "\n"
           "\x1b[4mOptions:\x1b[24m\n"
           "  -h, --help     Print help\n"
           "  -V, --version  Print version\n");
}

/**
 * @brief Print help message for `add` command
 *
 * @param is_error -- true: User did a wrong input, so we print an error message and the help message
 *                 -- false: User did nothing wrong, he just requested to print the help message
 */
static void help_add(bool is_error)
{
    if (is_error) {
        printf(ERROR_HOW_TO_USE);
    }

    printf("Add a new TODO\n"
           "\n"
           "\x1b[4mUsage:\x1b[24m todo add <TEXT> [OPTIONS]\n"
           "\n"
           "\x1b[4mArguments:\x1b[24m\n"
           "  <TEXT>  TODO text\n"
           "\n"
           "\x1b[4mOptions:\x1b[24m\n"
           "      --due <DATE>  Due date. One of:\n"
           "                      YYYY-MM-DD hh:mm:ss\n"
           "                      YYYY-MM-DD\n"
           "                      today\n"
           "                      tomorrow\n"
           "                      week\n"
           "                      month\n"
           "                      year\n"
           "  -h, --help        Print help\n");
}

/**
 * @brief Print help message for `list` command
 *
 * @param is_error -- true: User did a wrong input, so we print an error message and the help message
 *                 -- false: User did nothing wrong, he just requested to print the help message
 */
static void help_list(bool is_error)
{
    if (is_error) {
        printf(ERROR_HOW_TO_USE);
    }

    printf("List TODOs\n"
           "\n"
           "\x1b[4mUsage:\x1b[24m todo list [OPTIONS]\n"
           "\n"
           "\x1b[4mOptions:\x1b[24m\n"
           "      --overdue          List overdue TODOs\n"
           "      --all              List all TODOs, including completed\n"
           "      --until <DATE>     List TODOs due until DATE. One of:\n"
           "                           YYYY-MM-DD hh:mm:ss\n"
           "                           YYYY-MM-DD\n"
           "                           today\n"
           "                           tomorrow\n"
           "                           week\n"
           "                           month\n"
           "                           year\n"
           "      --search <TEXT>    List TODOs matching TEXT\n"
           "  -h, --help             Print help\n");
}

/**
 * @brief Print help message for `due` command
 *
 * @param is_error -- true: User did a wrong input, so we print an error message and the help message
 *                 -- false: User did nothing wrong, he just requested to print the help message
 */
static void help_due(bool is_error)
{
    if (is_error) {
        printf(ERROR_HOW_TO_USE);
    }

    printf("Set or change a TODO's due date\n"
           "\n"
           "\x1b[4mUsage:\x1b[24m todo due <ID> <DATE>\n"
           "\n"
           "\x1b[4mArguments:\x1b[24m\n"
           "  <ID>    TODO ID\n"
           "  <DATE>  Due date. One of:\n"
           "            YYYY-MM-DD hh:mm:ss\n"
           "            YYYY-MM-DD\n"
           "            today\n"
           "            tomorrow\n"
           "            week\n"
           "            month\n"
           "            year\n"
           "\n"
           "\x1b[4mOptions:\x1b[24m\n"
           "  -h, --help  Print help\n");
}

/**
 * @brief Print help message for `done` command
 *
 * @param is_error -- true: User did a wrong input, so we print an error message and the help message
 *                 -- false: User did nothing wrong, he just requested to print the help message
 */
static void help_done(bool is_error)
{
    if (is_error) {
        printf(ERROR_HOW_TO_USE);
    }

    printf("Mark a TODO as done\n"
           "\n"
           "\x1b[4mUsage:\x1b[24m todo done <ID>\n"
           "\n"
           "\x1b[4mArguments:\x1b[24m\n"
           "  <ID>  TODO ID\n"
           "\n"
           "\x1b[4mOptions:\x1b[24m\n"
           "  -h, --help  Print help\n");
}

/**
 * @brief Print help message for `review` command
 *
 * @param is_error -- true: User did a wrong input, so we print an error message and the help message
 *                 -- false: User did nothing wrong, he just requested to print the help message
 */
static void help_review(bool is_error)
{
    if (is_error) {
        printf(ERROR_HOW_TO_USE);
    }

    printf("Interactively review open TODOs\n"
           "\n"
           "\x1b[4mUsage:\x1b[24m todo review\n"
           "\n"
           "\x1b[4mOptions:\x1b[24m\n"
           "  -h, --help  Print help\n");
}
