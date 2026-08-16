#include "todo.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sqlite3.h"

#define DB_NAME "todo.db"

#define DB_CREATE_TABLE_IF_NOT_EXISTS "CREATE TABLE IF NOT EXISTS todos ("                      \
                                      "    id INTEGER PRIMARY KEY,"                             \
                                      "    text TEXT NOT NULL,"                                 \
                                      "    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP," \
                                      "    due_at TEXT,"                                        \
                                      "    done_at TEXT"                                        \
                                      ");"
#define DB_INSERT_INTO "INSERT INTO todos (text, due_at, done_at) VALUES ('%s', NULL, NULL);"
#define DB_SELECT_ALL  "SELECT id, text, created_at, due_at, done_at FROM todos;"

static sqlite3* db;

// +----------------------------------+
// | #1                               |
// | do something useful with my life |
// |                                  |
// | created = 2026-08-16 12:53:13    |
// | due     =                        |
// | done    =                        |
// +----------------------------------+
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

todo_error_t todo_add(char* text)
{
    char* zErrMsg  = 0;
    char  str[100] = "";

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }

    if (sqlite3_exec(db, DB_CREATE_TABLE_IF_NOT_EXISTS, callback, 0, &zErrMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sprintf(str, DB_INSERT_INTO, text);
    if (sqlite3_exec(db, str, callback, 0, &zErrMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);

    return TODO_ERROR_OK;
}

todo_error_t todo_list(void)
{
    char* zErrMsg = 0;

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }

    if (sqlite3_exec(db, DB_SELECT_ALL, callback, 0, &zErrMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);

    return TODO_ERROR_OK;
}

todo_error_t todo_review(void)
{
    // TODO: Select overdue todos from SQLite and print them one by one
    // For each todo, the user can device what to do (mark done, move to tomorrow, next week, next month..)

    return TODO_ERROR_OK;
}
