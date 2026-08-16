#ifndef TODO_H
#define TODO_H

typedef enum {
    TODO_ERROR_OK      = 0,
    TODO_ERROR_GENERAL = 1,
} todo_error_t;

todo_error_t todo_add(char* text);
todo_error_t todo_list(void);
todo_error_t todo_review(void);

#endif // TODO_H
