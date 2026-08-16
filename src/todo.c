#include "todo.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "sqlite3.h"

#define DB_NAME "todo.db"

#define DB_CREATE_TABLE_IF_NOT_EXISTS "CREATE TABLE IF NOT EXISTS todos ("                      \
                                      "    id INTEGER PRIMARY KEY,"                             \
                                      "    text TEXT NOT NULL,"                                 \
                                      "    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP," \
                                      "    due_at TEXT,"                                        \
                                      "    done_at TEXT"                                        \
                                      ");"
#define DB_INSERT_INTO          "INSERT INTO todos (text, due_at, done_at) VALUES ('%s', NULL, NULL);"
#define DB_INSERT_INTO_WITH_DUE "INSERT INTO todos (text, due_at, done_at) VALUES ('%s', '%s', NULL);"
#define DB_SELECT_ALL           "SELECT id, text, created_at, due_at, done_at FROM todos;"
#define DB_SELECT_DEFAULT       "SELECT id, text, created_at, due_at, done_at FROM todos WHERE done_at IS NULL;"
#define DB_SELECT_OVERDUE       "SELECT id, text, created_at, due_at, done_at FROM todos WHERE due_at <= CURRENT_TIMESTAMP AND done_at IS NULL;"
#define DB_UPDATE_DUE           "UPDATE todos SET due_at = '%s' WHERE id = %u;"
#define DB_UPDATE_DONE          "UPDATE todos SET done_at = CURRENT_TIMESTAMP WHERE id = %u;"

#define STR_LEN_MAX 256

static sqlite3* db;

static bool convert_to_iso(char* str, char** iso_str_ptr);
static int  callback(void* NotUsed, int argc, char** argv, char** azColName);

todo_error_t todo_add(int argc, char** argv)
{
    int   res              = 0;
    char* zErrMsg          = 0;
    char* due_date         = "";
    char* text             = "";
    char  str[STR_LEN_MAX] = "";

    if (argc < 1) {
        fprintf(stderr, "Not enough arguments");
        return TODO_ERROR_ARGUMENT;
    }

    text = argv[0];

    if (argc > 1) {
        if (strcmp(argv[1], "--due") == 0) {
            if (argc > 2) {
                if (!convert_to_iso(argv[2], &due_date)) {
                    fprintf(stderr, "Invalid argument format: %s\n", argv[2]);
                    return TODO_ERROR_ARGUMENT;
                }
            } else {
                fprintf(stderr, "Invalid number of arguments");
                return TODO_ERROR_ARGUMENT;
            }
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[1]);
            return TODO_ERROR_ARGUMENT;
        }
    }

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return TODO_ERROR_DB;
    }

    if (sqlite3_exec(db, DB_CREATE_TABLE_IF_NOT_EXISTS, callback, 0, &zErrMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return TODO_ERROR_DB;
    }

    if (due_date[0] == '\0') {
        res = snprintf(str, STR_LEN_MAX, DB_INSERT_INTO, text);
    } else {
        res = snprintf(str, STR_LEN_MAX, DB_INSERT_INTO_WITH_DUE, text, due_date);
    }

    if ((res < 0) || (res >= STR_LEN_MAX)) {
        fprintf(stderr, "SQL query could not be constructed, maybe string is too long; max. %d characters\n", STR_LEN_MAX);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return TODO_ERROR_ARGUMENT;
    }

    if (sqlite3_exec(db, str, callback, 0, &zErrMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return TODO_ERROR_DB;
    }

    sqlite3_free(zErrMsg);
    sqlite3_close(db);

    return TODO_ERROR_OK;
}

typedef enum {
    LIST_FLAG_DEFAULT,
    LIST_FLAG_OVERDUE,
    LIST_FLAG_ALL,
} list_flag_t;

todo_error_t todo_list(int argc, char** argv)
{
    list_flag_t flag             = LIST_FLAG_DEFAULT;
    char*       zErrMsg          = 0;
    char        str[STR_LEN_MAX] = "";

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[0], "--overdue") == 0) {
            flag = LIST_FLAG_OVERDUE;
        } else if (strcmp(argv[i], "--all") == 0) {
            flag = LIST_FLAG_ALL;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return TODO_ERROR_ARGUMENT;
        }
    }

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return TODO_ERROR_DB;
    }

    switch (flag) {
        case LIST_FLAG_DEFAULT:
            if (sqlite3_exec(db, DB_SELECT_DEFAULT, callback, 0, &zErrMsg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                sqlite3_close(db);
                return TODO_ERROR_DB;
            }
            break;

        case LIST_FLAG_OVERDUE:
            if (sqlite3_exec(db, DB_SELECT_OVERDUE, callback, 0, &zErrMsg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                sqlite3_close(db);
                return TODO_ERROR_DB;
            }
            break;

        case LIST_FLAG_ALL:
            if (sqlite3_exec(db, DB_SELECT_ALL, callback, 0, &zErrMsg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                sqlite3_close(db);
                return TODO_ERROR_DB;
            }
            break;

        default:
            sqlite3_free(zErrMsg);
            sqlite3_close(db);
            return TODO_ERROR_GENERAL;
    }

    sqlite3_free(zErrMsg);
    sqlite3_close(db);

    return TODO_ERROR_OK;
}

todo_error_t todo_due(uint32_t id, char* due_date)
{
    int   res              = 0;
    char* zErrMsg          = 0;
    char  str[STR_LEN_MAX] = "";

    if (!convert_to_iso(due_date, &due_date)) {
        fprintf(stderr, "Invalid argument format: %s\n", due_date);
        return TODO_ERROR_ARGUMENT;
    }

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return TODO_ERROR_DB;
    }

    res = snprintf(str, STR_LEN_MAX, DB_UPDATE_DUE, due_date, id);

    if ((res < 0) || (res >= STR_LEN_MAX)) {
        fprintf(stderr, "SQL query could not be constructed, maybe string is too long; max. %d characters\n", STR_LEN_MAX);
        sqlite3_close(db);
        return TODO_ERROR_ARGUMENT;
    }

    if (sqlite3_exec(db, str, callback, 0, &zErrMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return TODO_ERROR_DB;
    }

    sqlite3_free(zErrMsg);
    sqlite3_close(db);

    return TODO_ERROR_OK;
}

todo_error_t todo_done(uint32_t id)
{
    int   res              = 0;
    char* zErrMsg          = 0;
    char  str[STR_LEN_MAX] = "";

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return TODO_ERROR_DB;
    }

    res = snprintf(str, STR_LEN_MAX, DB_UPDATE_DONE, id);

    if ((res < 0) || (res >= STR_LEN_MAX)) {
        fprintf(stderr, "SQL query could not be constructed, maybe string is too long; max. %d characters\n", STR_LEN_MAX);
        sqlite3_close(db);
        return TODO_ERROR_ARGUMENT;
    }

    if (sqlite3_exec(db, str, callback, 0, &zErrMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return TODO_ERROR_DB;
    }

    sqlite3_free(zErrMsg);
    sqlite3_close(db);

    return TODO_ERROR_OK;
}

todo_error_t todo_review(void)
{
    // TODO: Select overdue todos from SQLite and print them one by one
    // For each todo, the user can device what to do (mark done, move to tomorrow, next week, next month..)
    printf("Interactive review not yet implemented\n");

    return TODO_ERROR_OK;
}

/**
 * @brief Callback from SQL query
 *
 * Prints each TODO, which matched the SQL query, in the following format:
 *
 * +----------------------------------+
 * | #1                               |
 * | do something useful with my life |
 * |                                  |
 * | created = 2026-08-16 12:53:13    |
 * | due     =                        |
 * | done    =                        |
 * +----------------------------------+
 *
 * @param NotUsed
 * @param argc
 * @param argv
 * @param azColName
 * @return int
 */
static int callback(void* NotUsed, int argc, char** argv, char** azColName)
{
    int MIN_WIDTH = 31;
    int width     = strlen(argv[1]) + 2;

    if (width < MIN_WIDTH) {
        width = MIN_WIDTH;
    }

    printf("+");
    for (int i = 0; i < width; i++) {
        printf("%c", '-');
    }
    printf("+\n");
    printf("| #%-*s |\n", width - 3, argv[0]);
    printf("| %-*s |\n", width - 2, argv[1]);
    printf("| %-*s |\n", width - 2, "");
    printf("| created = %-*s |\n", width - 12, argv[2]);
    printf("| due     = %-*s |\n", width - 12, argv[3] ? argv[3] : "");
    printf("| done    = %-*s |\n", width - 12, argv[4] ? argv[4] : "");
    printf("+");
    for (int i = 0; i < width; i++) {
        printf("%c", '-');
    }
    printf("+\n");

    return 0;
}

#define WITHIN(lower, value, upper) (((lower) <= (value)) && ((value) <= (upper)))

#define DATE_STR_LEN      (10)
#define DATETIME_STR_LEN  (19)
#define DATETIME_STR_SIZE (DATETIME_STR_LEN + 1)

/**
 * @brief Convert a string to ISO datetime format if it is valid
 *
 * @param str         -- input string
 * @param iso_str_ptr -- pointer to output string
 * @return true       -- input string is valid
 * @return false      -- input string is valid
 */
static bool convert_to_iso(char* str, char** iso_str_ptr)
{
    static char static_iso_str[DATETIME_STR_SIZE] = "";
    time_t      t                                 = time(NULL);
    struct tm   time_local                        = *localtime(&t);
    bool        keyword                           = true; // Where or not the user provided a keyword instead of an actual date

    if (strcmp(str, "today") == 0) {
        // nothing to do, `time` already holds todays date
    } else if (strcmp(str, "tomorrow") == 0) {
        time_local.tm_mday += 1;
    } else if (strcmp(str, "week") == 0) {
        time_local.tm_mday += 7;

    } else if (strcmp(str, "month") == 0) {
        time_local.tm_mon += 1;

    } else if (strcmp(str, "year") == 0) {
        time_local.tm_year += 1;

    } else {
        keyword = false;
    }

    if (keyword) {
        mktime(&time_local);
        snprintf(static_iso_str, DATETIME_STR_SIZE, "%04d-%02d-%02d %02d:%02d:%02d", time_local.tm_year + 1900, time_local.tm_mon + 1, time_local.tm_mday, 0, 0, 0);
        *iso_str_ptr = static_iso_str;
        return true;
    }

    // User provided a date or datetime

    if ((strlen(str) != DATE_STR_LEN) && (strlen(str) != DATETIME_STR_LEN)) {
        return false;
    }

    /* clang-format off */

    // YYYY-
    if (!WITHIN('0', str[0], '9')) return false;
    if (!WITHIN('0', str[1], '9')) return false;
    if (!WITHIN('0', str[2], '9')) return false;
    if (!WITHIN('0', str[3], '9')) return false;
    if (str[4] != '-') return false;

    // MM-
    if (!WITHIN('0', str[5], '1')) return false;
    if (str[5] == '1') {
        if (!WITHIN('0', str[6], '2')) return false;
    } else {
        if (!WITHIN('0', str[6], '9')) return false;
    }
    if (str[7] != '-') return false;

    // DD
    if (!WITHIN('0', str[8], '3')) return false;
    if (str[8] == '3') {
        if (!WITHIN('0', str[9], '1')) return false;
    } else {
        if (!WITHIN('0', str[9], '9')) return false;
    }

    for (int i = 0; i < DATE_STR_LEN; i++) {
        static_iso_str[i] = str[i];
    }

    if (strlen(str) == DATE_STR_LEN) {
         // Was just a date --> put time to zero
        snprintf(&static_iso_str[DATE_STR_LEN], DATETIME_STR_SIZE - DATE_STR_LEN, " %02d:%02d:%02d", 0, 0, 0);
        *iso_str_ptr = static_iso_str;
        return true;
    }

    // It is a date time --> continue checking...

    if (str[10] != ' ') return false;
    // hh:
    if (!WITHIN( '0', str[11], '2')) return false;
    if (str[11] == '2') {
        if (!WITHIN( '0', str[12], '3')) return false;
    } else {
        if (!WITHIN( '0', str[12], '9')) return false;
    }
    if (str[13] != ':') return false;

    // mm:
    if (!WITHIN( '0', str[14], '5')) return false;
    if (!WITHIN( '0', str[15], '9')) return false;
    if (str[16] != ':') return false;

    // ss
    if (!WITHIN( '0', str[17], '5')) return false;
    if (!WITHIN( '0', str[18], '9')) return false;

    /* clang-format on */

    for (int i = DATE_STR_LEN; i < DATETIME_STR_LEN; i++) {
        static_iso_str[i] = str[i];
    }
    static_iso_str[DATETIME_STR_LEN] = '\0';

    *iso_str_ptr = static_iso_str;

    return true;
}
