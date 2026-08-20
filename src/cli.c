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

#define HELP(argc, argv)     (((argc) > 1) && ((strcmp((argv)[1], "--help") == 0) || (strcmp((argv)[1], "-h") == 0) || (strcmp((argv)[1], "help") == 0)))
#define HELP_CMD(argc, argv) (((argc) > 2) && ((strcmp((argv)[2], "--help") == 0) || (strcmp((argv)[2], "-h") == 0)))

int main(int argc, char** argv)
{
    todo_error_t error = TODO_ERROR_OK;

    if (argc < 2) {
        help(true);
        return TODO_ERROR_ARGUMENT;
    }

    if ((strcmp(argv[1], "--version") == 0) || (strcmp(argv[1], "-V") == 0)) {
        printf("v" TODO_VERSION "\n");
    } else if (HELP(argc, argv)) {
        help(false);
    } else if (strcmp(argv[1], "add") == 0) {
        if (HELP_CMD(argc, argv)) {
            help_add(false);
            return TODO_ERROR_OK;
        }

        if (argc < 3) {
            help_add(true);
            return TODO_ERROR_ARGUMENT;
        }

        error = todo_add(argc - 2, &argv[2]);
        if (error == TODO_ERROR_ARGUMENT) {
            help_add(true);
        }
    } else if (strcmp(argv[1], "list") == 0) {
        if (HELP_CMD(argc, argv)) {
            help_list(false);
            return TODO_ERROR_OK;
        }

        if (argc < 2) {
            help_list(true);
            return TODO_ERROR_ARGUMENT;
        }

        error = todo_list(argc - 2, &argv[2]);
        if (error == TODO_ERROR_ARGUMENT) {
            help_add(true);
        }
    } else if (strcmp(argv[1], "due") == 0) {
        if (HELP_CMD(argc, argv)) {
            help_due(false);
            return TODO_ERROR_OK;
        }

        if (argc != 4) {
            help_due(true);
            return TODO_ERROR_ARGUMENT;
        }

        int32_t id = atoi(argv[2]);
        if (id <= 0) {
            help_due(true);
            return TODO_ERROR_ARGUMENT;
        }

        error = todo_due((uint32_t)id, argv[3]);
        if (error == TODO_ERROR_ARGUMENT) {
            help_add(true);
        }
    } else if (strcmp(argv[1], "done") == 0) {
        if (HELP_CMD(argc, argv)) {
            help_done(false);
            return TODO_ERROR_OK;
        }

        if (argc != 3) {
            help_done(true);
            return TODO_ERROR_ARGUMENT;
        }

        int32_t id = atoi(argv[2]);
        if (id <= 0) {
            help_done(true);
            return TODO_ERROR_ARGUMENT;
        }

        error = todo_done((uint32_t)id);
        if (error == TODO_ERROR_ARGUMENT) {
            help_add(true);
        }
    } else if (strcmp(argv[1], "review") == 0) {
        if (HELP_CMD(argc, argv)) {
            help_review(false);
            return TODO_ERROR_OK;
        }

        if (argc != 2) {
            help_review(true);
            return TODO_ERROR_ARGUMENT;
        }

        error = todo_review();
        if (error == TODO_ERROR_ARGUMENT) {
            help_add(true);
        }
    } else {
        help(true);
        error = TODO_ERROR_ARGUMENT;
    }

    return error;
}

#define ERROR_HOW_TO_USE "ERROR, this is how to use:\n\n"

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
