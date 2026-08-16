#include <stdio.h>
#include <string.h>

#include "todo.h"

static void how_to_use(void);
static void how_to_use_add(void);
static void how_to_use_list(void);
static void how_to_use_review(void);

int main(int argc, char** argv)
{
    if (argc < 2) {
        how_to_use();
        return 1;
    }

    if (strcmp(argv[1], "add") == 0) {
        if (argc != 3) {
            how_to_use_add();
            return 1;
        }
        return todo_add(argv[2]);
    }

    if (strcmp(argv[1], "list") == 0) {
        if (argc != 2) {
            how_to_use_list();
            return 1;
        }
        return todo_list();
    }

    if (strcmp(argv[1], "review") == 0) {
        if (argc != 2) {
            how_to_use_review();
            return 1;
        }
        return todo_review();
    }

    return 0;
}

static void how_to_use(void)
{
    printf("ERROR, this is how to use: `todo <cmd>`");
}

static void how_to_use_add(void)
{
    printf("ERROR, this is how to use: `todo add <text>`");
}

static void how_to_use_list(void)
{
    printf("ERROR, this is how to use: `todo list`");
}

static void how_to_use_review(void)
{
    printf("ERROR, this is how to use: `todo review`");
}
