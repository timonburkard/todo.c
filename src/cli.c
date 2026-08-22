#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "clic.h"
#include "todo.h"
#include "version.h"

typedef enum {
    CMD_ID_ADD = 0,
    CMD_ID_LIST,
    CMD_ID_DUE,
    CMD_ID_DONE,
    CMD_ID_REVIEW,
} cmd_id_t;

typedef enum {
    ADD_ARG_ID_TEXT = 0,
    ADD_ARG_ID_DUE,
    LIST_ARG_ID_OVERDUE = 0,
    LIST_ARG_ID_ALL,
    LIST_ARG_ID_UNTIL,
    LIST_ARG_ID_SEARCH,
    DUE_ARG_ID_ID = 0,
    DUE_ARG_ID_DATE,
    DONE_ARG_ID_ID = 0
} arg_id_t;

static clic_err_t add(clic_res_t* clic_res);
static clic_err_t list(clic_res_t* clic_res);
static clic_err_t due(clic_res_t* clic_res);
static clic_err_t done(clic_res_t* clic_res);
static clic_err_t review(clic_res_t* clic_res);

static uint32_t   str_to_id(const char* str);
static clic_err_t todo_to_clic_err(todo_error_t todo_error);

typedef struct {
    const char* name;
    todo_error_t (*function)(int, char**);
    void (*helper)(bool);
} cmd_t;

static const clic_cmd_t cmdv[] = {
    [CMD_ID_ADD] = {
        .names       = (const char*[]){"add", NULL},
        .description = "Add a new TODO",
        .function    = add,
        .argc        = 2,
        .argv        = (const clic_arg_t[]){
            [ADD_ARG_ID_TEXT] = {
                       .type        = CLIC_ARG_POSITIONAL,
                       .required    = true,
                       .value_name  = "TEXT",
                       .description = "Description of the new TODO",
            },
            [ADD_ARG_ID_DUE] = {
                       .type        = CLIC_ARG_WITH_VALUE,
                       .required    = false,
                       .names       = (const char*[]){"--due", NULL},
                       .value_name  = "DATE",
                       .description = "Due date. One of: YYYY-MM-DD hh:mm:ss, YYYY-MM-DD, today, tomorrow, week, month, year",
            },
        },
    },
    [CMD_ID_LIST] = {
        .names       = (const char*[]){"list", NULL},
        .description = "List TODOs",
        .function    = list,
        .argc        = 4,
        .argv        = (const clic_arg_t[]){
            [LIST_ARG_ID_OVERDUE] = {
                       .type        = CLIC_ARG_FLAG,
                       .required    = false,
                       .names       = (const char*[]){"--overdue", NULL},
                       .description = "List overdue TODOs",
            },
            [LIST_ARG_ID_ALL] = {
                       .type        = CLIC_ARG_FLAG,
                       .required    = false,
                       .names       = (const char*[]){"--all", "-a", NULL},
                       .description = " List all TODOs, including the completed ones",
            },
            [LIST_ARG_ID_UNTIL] = {
                       .type        = CLIC_ARG_WITH_VALUE,
                       .required    = false,
                       .names       = (const char*[]){"--until", NULL},
                       .value_name  = "DATE",
                       .description = "List TODOs which are due until DATE. One of: YYYY-MM-DD hh:mm:ss, YYYY-MM-DD, today, tomorrow, week, month, year",
            },
            [LIST_ARG_ID_SEARCH] = {
                       .type        = CLIC_ARG_WITH_VALUE,
                       .required    = false,
                       .names       = (const char*[]){"--search", NULL},
                       .value_name  = "TEXT",
                       .description = "List TODOs which contain TEXT",
            },
        },
    },
    [CMD_ID_DUE] = {
        .names       = (const char*[]){"due", NULL},
        .description = "Set or change a TODO's due date",
        .function    = due,
        .argc        = 2,
        .argv        = (const clic_arg_t[]){
            [DUE_ARG_ID_ID] = {
                       .type        = CLIC_ARG_POSITIONAL,
                       .required    = true,
                       .value_name  = "ID",
                       .description = "ID of the TODO",
            },
            [DUE_ARG_ID_DATE] = {
                       .type        = CLIC_ARG_POSITIONAL,
                       .required    = true,
                       .value_name  = "DATE",
                       .description = "Due date. One of: YYYY-MM-DD hh:mm:ss, YYYY-MM-DD, today, tomorrow, week, month, year",
            },
        },
    },
    [CMD_ID_DONE] = {
        .names       = (const char*[]){"done", NULL},
        .description = "Mark a TODO as done",
        .function    = done,
        .argc        = 1,
        .argv        = (const clic_arg_t[]){
            [DONE_ARG_ID_ID] = {
                       .type        = CLIC_ARG_POSITIONAL,
                       .required    = true,
                       .description = "ID of the TODO",
            },
        },
    },
    [CMD_ID_REVIEW] = {
        .names       = (const char*[]){"review", NULL},
        .description = "Interactively review open",
        .function    = review,
        .argc        = 0,
    },
};

int main(int argc, char** argv)
{
    clic_err_t clic_err;

    uint8_t cmdc = sizeof(cmdv) / sizeof(cmdv[0]);

    clic_err = clic_parse(cmdc, cmdv, argc, (const char* const*)argv);

    switch (clic_err) {
        case CLIC_ERR_OK:
            // Nothing to do
            break;

        case CLIC_ERR_CONFIG:
            printf("ERROR: Wrong CLIC configuration!\n");
            break;
        case CLIC_ERR_NULL:
            printf("ERROR: NULL pointer provided to CLIC!\n");
            break;
        case CLIC_ERR_GENERAL:
            printf("ERROR: General CLIC error code!\n");
            break;

        case CLIC_ERR_ARG:
            // Nothing to print because CLIC already prints error for wrong arguments
            break;

        default:
            UNREACHABLE();
    }

    return (int)clic_err;
}

/**
 * @brief Handle `add` command
 *
 * @param clic_res -- Parses CLIC result
 *
 * @return clic_err_t -- CLIC error code
 */
static clic_err_t add(clic_res_t* clic_res)
{
    todo_error_t error = todo_add(clic_res->argv[ADD_ARG_ID_TEXT], clic_res->argv[ADD_ARG_ID_DUE]);

    return todo_to_clic_err(error);
}

/**
 * @brief Handle `list` command
 *
 * @param clic_res -- Parses CLIC result
 *
 * @return clic_err_t -- CLIC error code
 */
static clic_err_t list(clic_res_t* clic_res)
{
    todo_error_t  error    = TODO_ERROR_OK;
    todo_filter_t filter   = TODO_FILTER_DEFAULT;
    char*         argument = NULL;

    if (clic_res->argv[LIST_ARG_ID_OVERDUE] != NULL) {
        filter = TODO_FILTER_OVERDUE;
    } else if (clic_res->argv[LIST_ARG_ID_ALL] != NULL) {
        filter = TODO_FILTER_ALL;
    } else if (clic_res->argv[LIST_ARG_ID_UNTIL] != NULL) {
        filter   = TODO_FILTER_UNTIL;
        argument = clic_res->argv[LIST_ARG_ID_UNTIL];
    } else if (clic_res->argv[LIST_ARG_ID_SEARCH] != NULL) {
        filter   = TODO_FILTER_SEARCH;
        argument = clic_res->argv[LIST_ARG_ID_SEARCH];
    }

    error = todo_list(filter, argument);

    return todo_to_clic_err(error);
}

/**
 * @brief Handle `due` command
 *
 * @param clic_res -- Parses CLIC result
 *
 * @return clic_err_t -- CLIC error code
 */
static clic_err_t due(clic_res_t* clic_res)
{
    uint32_t     id;
    todo_error_t error;

    id = str_to_id(clic_res->argv[DUE_ARG_ID_ID]);

    if (id == 0) {
        return CLIC_ERR_ARG;
    }

    error = todo_due(id, clic_res->argv[DUE_ARG_ID_DATE]);

    return todo_to_clic_err(error);
}

/**
 * @brief Handle `done` command
 *
 * @param clic_res -- Parses CLIC result
 *
 * @return clic_err_t -- CLIC error code
 */
static clic_err_t done(clic_res_t* clic_res)
{
    uint32_t     id;
    todo_error_t error;

    id = str_to_id(clic_res->argv[DONE_ARG_ID_ID]);

    if (id == 0) {
        return CLIC_ERR_ARG;
    }

    error = todo_done(id);

    return todo_to_clic_err(error);
}

/**
 * @brief Handle `review` command
 *
 * @param clic_res -- Parses CLIC result
 *
 * @return clic_err_t -- CLIC error code
 */
static clic_err_t review(clic_res_t* clic_res)
{
    todo_error_t error;

    (void)clic_res; // no arguments

    error = todo_review();

    return todo_to_clic_err(error);
}

static clic_err_t todo_to_clic_err(todo_error_t todo_error)
{
    switch (todo_error) {
        case TODO_ERROR_OK:
            return CLIC_ERR_OK;

        case TODO_ERROR_ARGUMENT:
            return CLIC_ERR_ARG;

        default:
            return CLIC_ERR_GENERAL;
    }
}

/**
 * @brief Convert string to ID
 *
 * Valid ID is uint32_t and none 0.
 * In case of invalid string, 0 is returned.
 *
 * @param[in] str -- String to be converted
 *
 * @return uint32_t -- Converted ID
 */
static uint32_t str_to_id(const char* str)
{
    char*         end;
    unsigned long result;

    if (str == NULL) {
        return 0;
    }

    if ((*str == '-') || (*str == '+')) {
        return 0; // We don't accept sign
    }

    errno = 0;

    result = strtoul(str, &end, 10);

    if (errno == ERANGE) {
        return 0;
    }

    if (end == str) {
        return 0;
    }

    if (*end != '\0') {
        return 0;
    }

    if ((result == 0) || (result > UINT32_MAX)) {
        return 0;
    }

    return (uint32_t)result;
}
