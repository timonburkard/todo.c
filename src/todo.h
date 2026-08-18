#ifndef TODO_H
#define TODO_H

#include <stdint.h>

typedef enum {
    TODO_ERROR_OK       = 0,
    TODO_ERROR_GENERAL  = 1,
    TODO_ERROR_ARGUMENT = 2,
    TODO_ERROR_SQL      = 3,
} todo_error_t;

todo_error_t todo_add(int argc, char** argv);
todo_error_t todo_list(int argc, char** argv);
todo_error_t todo_due(uint32_t id, char* due_date);
todo_error_t todo_done(uint32_t id);
todo_error_t todo_review(void);

#endif // TODO_H
