#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "todo.h"

static void how_to_use(void);
static void how_to_use_add(void);
static void how_to_use_list(void);
static void how_to_use_due(void);
static void how_to_use_done(void);
static void how_to_use_review(void);

int main(int argc, char** argv)
{
    todo_error_t error = TODO_ERROR_OK;

    if (argc < 2) {
        how_to_use();
        return TODO_ERROR_ARGUMENT;
    }

    if (strcmp(argv[1], "add") == 0) {
        if (argc < 3) {
            how_to_use_add();
            return TODO_ERROR_ARGUMENT;
        }
        error = todo_add(argc - 2, &argv[2]);
    } else if (strcmp(argv[1], "list") == 0) {
        if (argc < 2) {
            how_to_use_list();
            return TODO_ERROR_ARGUMENT;
        }
        error = todo_list(argc - 2, &argv[2]);
    } else if (strcmp(argv[1], "due") == 0) {
        if (argc != 4) {
            how_to_use_due();
            return TODO_ERROR_ARGUMENT;
        }
        uint32_t id = atoi(argv[2]);
        if (id == 0) {
            how_to_use_due();
            return TODO_ERROR_ARGUMENT;
        }
        error = todo_due(id, argv[3]);
    } else if (strcmp(argv[1], "done") == 0) {
        if (argc != 3) {
            how_to_use_done();
            return TODO_ERROR_ARGUMENT;
        }
        uint32_t id = atoi(argv[2]);
        if (id == 0) {
            how_to_use_done();
            return TODO_ERROR_ARGUMENT;
        }
        error = todo_done(id);
    } else if (strcmp(argv[1], "review") == 0) {
        if (argc != 2) {
            how_to_use_review();
            return TODO_ERROR_ARGUMENT;
        }
        error = todo_review();
    } else {
        how_to_use();
        return TODO_ERROR_ARGUMENT;
    }

    return error;
}

static void how_to_use(void)
{
    printf("ERROR, this is how to use: todo <cmd>\nAvailable <cmd>: add, list, review\n");
}

static void how_to_use_add(void)
{
    printf("ERROR, this is how to use: todo add <text>\n");
}

static void how_to_use_list(void)
{
    printf("ERROR, this is how to use: todo list [--all]\n");
}

static void how_to_use_due(void)
{
    printf("ERROR, this is how to use: todo due <id> <date>\n");
}

static void how_to_use_done(void)
{
    printf("ERROR, this is how to use: todo done <id>\n");
}

static void how_to_use_review(void)
{
    printf("ERROR, this is how to use: todo review\n");
}
