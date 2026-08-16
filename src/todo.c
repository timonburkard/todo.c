#include "todo.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
#define DB_INSERT_INTO                 "INSERT INTO todos (text, due_at, done_at) VALUES ('%s', NULL, NULL);"
#define DB_INSERT_INTO_WITH_DUE        "INSERT INTO todos (text, due_at, done_at) VALUES ('%s', '%s', NULL);"
#define DB_SELECT_ALL                  "SELECT id, text, created_at, due_at, done_at FROM todos;"
#define DB_SELECT_ID                   "SELECT id, text, created_at, due_at, done_at FROM todos WHERE id = %u;"
#define DB_SELECT_DEFAULT              "SELECT id, text, created_at, due_at, done_at FROM todos WHERE done_at IS NULL;"
#define DB_SELECT_DEFAULT_ORDER_BY_DUE "SELECT id, text, created_at, due_at, done_at FROM todos WHERE done_at IS NULL ORDER BY due_at ASC;"
#define DB_SELECT_OVERDUE              "SELECT id, text, created_at, due_at, done_at FROM todos WHERE due_at <= CURRENT_TIMESTAMP AND done_at IS NULL;"
#define DB_SELECT_UNTIL                "SELECT id, text, created_at, due_at, done_at FROM todos WHERE due_at <= '%s' AND done_at IS NULL;"
#define DB_SELECT_SEARCH               "SELECT id, text, created_at, due_at, done_at FROM todos WHERE text LIKE '%%%s%%';"
#define DB_UPDATE_DUE                  "UPDATE todos SET due_at = '%s' WHERE id = %u;"
#define DB_UPDATE_DONE                 "UPDATE todos SET done_at = CURRENT_TIMESTAMP WHERE id = %u;"
#define DB_DELETE                      "DELETE FROM todos where id = %u;"

#define STR_LEN_MAX 256

#define UNREACHABLE()                                                               \
    do {                                                                            \
        fprintf(stderr, "UNREACHABLE code reached at %s:%d\n", __FILE__, __LINE__); \
        abort();                                                                    \
    } while (0)

typedef enum {
    ROUND_DOWN,
    ROUND_UP,
} round_t;

static sqlite3* db;

#define CALLBACK_IDS_MAX 256
static volatile uint16_t callback_ids_amount            = 0;
static volatile uint32_t callback_ids[CALLBACK_IDS_MAX] = {0};

static void print_todo(char* id, char* text, char* created_at, char* due_at, char* done_at);
static bool convert_to_iso(char* str, char** iso_str_ptr, round_t rounding);
static int  callback_print(void* NotUsed, int argc, char** argv, char** azColName);
static int  callback_get_id(void* NotUsed, int argc, char** argv, char** azColName);

todo_error_t todo_add(int argc, char** argv)
{
    int   res              = 0;
    char* zErrMsg          = NULL;
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
                if (!convert_to_iso(argv[2], &due_date, ROUND_DOWN)) {
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

    if (sqlite3_exec(db, DB_CREATE_TABLE_IF_NOT_EXISTS, NULL, 0, &zErrMsg) != SQLITE_OK) {
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

    if (sqlite3_exec(db, str, NULL, 0, &zErrMsg) != SQLITE_OK) {
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
    LIST_FLAG_UNTIL,
    LIST_FLAG_ALL,
    LIST_FLAG_SEARCH,
} list_flag_t;

todo_error_t todo_list(int argc, char** argv)
{
    char        sql_str[STR_LEN_MAX] = "";
    list_flag_t flag                 = LIST_FLAG_DEFAULT;
    int         res                  = 0;
    char*       zErrMsg              = NULL;
    char*       until_str            = "";
    char*       search_str           = "";

    if (argc == 1) {
        if (strcmp(argv[0], "--overdue") == 0) {
            flag = LIST_FLAG_OVERDUE;
        } else if (strcmp(argv[0], "--all") == 0) {
            flag = LIST_FLAG_ALL;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[0]);
            return TODO_ERROR_ARGUMENT;
        }
    } else if (argc == 2) {
        if (strcmp(argv[0], "--until") == 0) {
            flag = LIST_FLAG_UNTIL;
            if (!convert_to_iso(argv[1], &until_str, ROUND_UP)) {
                fprintf(stderr, "Invalid argument format: %s\n", argv[1]);
                return TODO_ERROR_ARGUMENT;
            }
        } else if (strcmp(argv[0], "--search") == 0) {
            flag       = LIST_FLAG_SEARCH;
            search_str = argv[1];
        } else {
            fprintf(stderr, "Unknown argument: %s %s\n", argv[0], argv[1]);
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
            if (sqlite3_exec(db, DB_SELECT_DEFAULT, callback_print, 0, &zErrMsg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                sqlite3_close(db);
                return TODO_ERROR_DB;
            }
            break;

        case LIST_FLAG_OVERDUE:
            if (sqlite3_exec(db, DB_SELECT_OVERDUE, callback_print, 0, &zErrMsg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                sqlite3_close(db);
                return TODO_ERROR_DB;
            }
            break;

        case LIST_FLAG_UNTIL:
            res = snprintf(sql_str, STR_LEN_MAX, DB_SELECT_UNTIL, until_str);

            if ((res < 0) || (res >= STR_LEN_MAX)) {
                fprintf(stderr, "SQL query could not be constructed, maybe string is too long; max. %d characters\n", STR_LEN_MAX);
                sqlite3_close(db);
                return TODO_ERROR_ARGUMENT;
            }

            if (sqlite3_exec(db, sql_str, callback_print, 0, &zErrMsg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                sqlite3_close(db);
                return TODO_ERROR_DB;
            }
            break;

        case LIST_FLAG_ALL:
            if (sqlite3_exec(db, DB_SELECT_ALL, callback_print, 0, &zErrMsg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                sqlite3_close(db);
                return TODO_ERROR_DB;
            }
            break;

        case LIST_FLAG_SEARCH:
            res = snprintf(sql_str, STR_LEN_MAX, DB_SELECT_SEARCH, search_str);

            if ((res < 0) || (res >= STR_LEN_MAX)) {
                fprintf(stderr, "SQL query could not be constructed, maybe string is too long; max. %d characters\n", STR_LEN_MAX);
                sqlite3_close(db);
                return TODO_ERROR_ARGUMENT;
            }

            if (sqlite3_exec(db, sql_str, callback_print, 0, &zErrMsg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                sqlite3_close(db);
                return TODO_ERROR_DB;
            }
            break;

        default:
            UNREACHABLE();
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
    char* zErrMsg          = NULL;
    char  str[STR_LEN_MAX] = "";

    if (!convert_to_iso(due_date, &due_date, ROUND_DOWN)) {
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

    if (sqlite3_exec(db, str, NULL, 0, &zErrMsg) != SQLITE_OK) {
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
    char* zErrMsg          = NULL;
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

    if (sqlite3_exec(db, str, NULL, 0, &zErrMsg) != SQLITE_OK) {
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
    bool     is_selected          = false;
    int      res                  = 0;
    uint32_t id                   = 0;
    char     action               = '\0';
    char     due_str[STR_LEN_MAX] = "";
    char*    zErrMsg              = NULL;
    char*    due_iso_str          = NULL;
    char     sql_str[STR_LEN_MAX] = "";

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return TODO_ERROR_DB;
    }

    callback_ids_amount = 0;

    if (sqlite3_exec(db, DB_SELECT_DEFAULT_ORDER_BY_DUE, callback_get_id, 0, &zErrMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return TODO_ERROR_DB;
    }

    for (uint16_t i = 0; i < callback_ids_amount; i++) {
        id = callback_ids[i];

        // Print current TODO

        res = snprintf(sql_str, STR_LEN_MAX, DB_SELECT_ID, id);

        if ((res < 0) || (res >= STR_LEN_MAX)) {
            fprintf(stderr, "SQL query could not be constructed, maybe string is too long; max. %d characters\n", STR_LEN_MAX);
            sqlite3_close(db);
            return TODO_ERROR_ARGUMENT;
        }

        if (sqlite3_exec(db, sql_str, callback_print, 0, &zErrMsg) != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            sqlite3_close(db);
            return TODO_ERROR_DB;
        }

        do {
            printf("Please choose action: S(kip), D(one), M(ove), R(emove), A(bort)\n");
            scanf(" %c", &action);
            getchar(); // consume '\n'

            switch (action) {
                // Abort
                case 'A':
                case 'a':
                    sqlite3_free(zErrMsg);
                    sqlite3_close(db);
                    return TODO_ERROR_OK;

                // Skip
                case 'S':
                case 's':
                    is_selected = true;
                    break;

                // Done
                case 'D':
                case 'd':
                    res         = snprintf(sql_str, STR_LEN_MAX, DB_UPDATE_DONE, id);
                    is_selected = true;
                    break;

                // Remove
                case 'R':
                case 'r':
                    res         = snprintf(sql_str, STR_LEN_MAX, DB_DELETE, id);
                    is_selected = true;
                    break;

                // Moves
                case 'M':
                case 'm':
                    do {
                        printf("Please choose new due date:\n");
                        if (fgets(due_str, STR_LEN_MAX, stdin) == NULL) {
                            printf("String too long!\n");
                            continue;
                        }

                        due_str[strcspn(due_str, "\n")] = '\0';

                        if (convert_to_iso(due_str, &due_iso_str, ROUND_DOWN)) {
                            is_selected = true;
                        } else {
                            printf("Invalid format!\n");
                        }
                    } while (!is_selected);

                    res         = snprintf(sql_str, STR_LEN_MAX, DB_UPDATE_DUE, due_iso_str, id);
                    is_selected = true;
                    break;

                default:
                    is_selected = false;
                    printf("Invalid selection!\n");
                    break;
            }
        } while (!is_selected);

        if ((action == 's') || (action == 'S')) {
            continue;
        }

        if ((res < 0) || (res >= STR_LEN_MAX)) {
            fprintf(stderr, "SQL query could not be constructed, maybe string is too long; max. %d characters\n", STR_LEN_MAX);
            sqlite3_free(zErrMsg);
            sqlite3_close(db);
            return TODO_ERROR_ARGUMENT;
        }

        if (sqlite3_exec(db, sql_str, NULL, 0, &zErrMsg) != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            sqlite3_close(db);
            return TODO_ERROR_DB;
        }
    }

    sqlite3_free(zErrMsg);
    sqlite3_close(db);

    return TODO_ERROR_OK;
}

/**
 * @brief Print TODO in the following format:
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
 * @param id
 * @param text
 * @param created_at
 * @param due_at
 * @param done_at
 */
static void print_todo(char* id, char* text, char* created_at, char* due_at, char* done_at)
{
    const size_t MIN_WIDTH = 31;
    size_t       width     = strlen(text) + 2;

    if (width < MIN_WIDTH) {
        width = MIN_WIDTH;
    }

    printf("+");
    for (size_t i = 0; i < width; i++) {
        printf("%c", '-');
    }
    printf("+\n");
    printf("| #%-*s |\n", (int)(width - 3), id);
    printf("| %-*s |\n", (int)(width - 2), text);
    printf("| %-*s |\n", (int)(width - 2), "");
    printf("| created = %-*s |\n", (int)(width - 12), created_at);
    printf("| due     = %-*s |\n", (int)(width - 12), due_at ? due_at : "");
    printf("| done    = %-*s |\n", (int)(width - 12), done_at ? done_at : "");
    printf("+");
    for (size_t i = 0; i < width; i++) {
        printf("%c", '-');
    }
    printf("+\n");
}

/**
 * @brief Callback from SQL query, which prints the TODO
 *
 * Prints each TODO, which matched the SQL query, in the following format:
 *
 * @param NotUsed
 * @param argc
 * @param argv
 * @param azColName
 * @return int
 */
static int callback_print(void* NotUsed, int argc, char** argv, char** azColName)
{
    (void)NotUsed;
    (void)azColName;

    if (argc < 5) {
        UNREACHABLE();
        return 1;
    }

    print_todo(argv[0], argv[1], argv[2], argv[3], argv[4]);

    return 0;
}

/**
 * @brief Callback from SQL query, which collects the ID
 *
 * @param NotUsed
 * @param argc
 * @param argv
 * @param azColName
 * @return int
 */
static int callback_get_id(void* NotUsed, int argc, char** argv, char** azColName)
{
    (void)NotUsed;
    (void)azColName;

    if (argc < 1) {
        UNREACHABLE();
        return 1;
    }

    if (callback_ids_amount < CALLBACK_IDS_MAX) {
        int32_t id = atoi(argv[0]);

        if (id <= 0) {
            UNREACHABLE();
            return 1;
        }

        callback_ids[callback_ids_amount] = (uint32_t)id;
        ++callback_ids_amount;

        return 0;
    }

    return 1;
}

#define WITHIN(lower, value, upper) (((lower) <= (value)) && ((value) <= (upper)))

#define DATE_STR_LEN      (10)
#define DATETIME_STR_LEN  (19)
#define DATETIME_STR_SIZE (DATETIME_STR_LEN + 1)

/**
 * @brief Convert a string to ISO datetime format if it is valid
 *
 * @p str must have one of these ISO formats:
 *  - ISO datetime: "YYYY-MM-DD hh:mm:ss"
 *  - ISO date:     "YYYY-MM-DD"
 * Or be one of these keywords:
 *  - Today:        "today"
 *  - Tomorrow:     "tomorrow"
 *  - In a week:    "week"
 *  - In a month:   "month"
 *  - In a year:    "year"
 *
 * @param str         -- input string
 * @param iso_str_ptr -- pointer to output string
 * @param rounding    -- Round up or down for keywords
 * @return true       -- input string is valid
 * @return false      -- input string is invalid
 */
static bool convert_to_iso(char* str, char** iso_str_ptr, round_t rounding)
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
        if (rounding == ROUND_DOWN) {
            snprintf(static_iso_str, DATETIME_STR_SIZE, "%04u-%02u-%02u %02u:%02u:%02u", time_local.tm_year + 1900, time_local.tm_mon + 1, time_local.tm_mday, 0, 0, 0);
        } else if (rounding == ROUND_UP) {
            snprintf(static_iso_str, DATETIME_STR_SIZE, "%04u-%02u-%02u %02u:%02u:%02u", time_local.tm_year + 1900, time_local.tm_mon + 1, time_local.tm_mday, 23, 59, 59);
        } else {
            UNREACHABLE();
            return false;
        }

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

    /* clang-format on */

    for (int i = 0; i < DATE_STR_LEN; i++) {
        static_iso_str[i] = str[i];
    }

    if (strlen(str) == DATE_STR_LEN) {
        // Was just a date
        if (rounding == ROUND_DOWN) {
            snprintf(&static_iso_str[DATE_STR_LEN], DATETIME_STR_SIZE - DATE_STR_LEN, " %02d:%02d:%02d", 0, 0, 0);
        } else if (rounding == ROUND_UP) {
            snprintf(&static_iso_str[DATE_STR_LEN], DATETIME_STR_SIZE - DATE_STR_LEN, " %02d:%02d:%02d", 23, 59, 59);
        } else {
            UNREACHABLE();
        }

        *iso_str_ptr = static_iso_str;
        return true;
    }

    // It is a date time --> continue checking...

    /* clang-format off */

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
