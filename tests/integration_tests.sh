#!/bin/bash

# Integration tests for todo CLI
# Tests the full application workflow

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
TODO_BIN="$PROJECT_ROOT/todo"
TEST_DB="$PROJECT_ROOT/test_todo.db"

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0

# Helper function to run a test
run_test() {
    local test_name="$1"
    local expected_status="$2"
    shift 2

    echo -n "Testing: $test_name ... "

    # Run command and capture exit code (don't exit on failure)
    "$@" > /dev/null 2>&1
    actual_status=$?

    if [ "$actual_status" -eq "$expected_status" ]; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC} (expected $expected_status, got $actual_status)"
        ((FAILED++))
    fi
}

# Cleanup function
cleanup() {
    cd "$PROJECT_ROOT" 2>/dev/null || true
    rm -rf "$TEMP_DIR" 2>/dev/null || true
}
trap cleanup EXIT

# Clean up before tests
rm -f "$TEST_DB" || true

# Since the app uses hardcoded DB_NAME, we need to test in a temp directory
TEMP_DIR=$(mktemp -d) || { echo "Failed to create temp directory"; exit 1; }
cd "$TEMP_DIR" || { echo "Failed to cd to temp directory"; exit 1; }

# Test 1: Add a simple todo
run_test "add simple todo" 0 "$TODO_BIN" add "Buy milk"

# Test 2: Add another todo
run_test "add second todo" 0 "$TODO_BIN" add "Walk the dog"

# Test 3: List todos (should show incomplete only)
run_test "list incomplete todos" 0 "$TODO_BIN" list

# Test 4: List all todos
run_test "list all todos" 0 "$TODO_BIN" list --all

# Test 5: Add todo with due date
run_test "add todo with due date" 0 "$TODO_BIN" add "Fix bug" --due "2026-08-20 10:00:00"

# Test 6: Set due date for first todo
run_test "set due date for todo" 0 "$TODO_BIN" due 1 "2026-08-25 15:30:00"

# Test 7: Mark todo as done
run_test "mark todo as done" 0 "$TODO_BIN" done 1

# Test 8: Invalid date format should fail
run_test "reject invalid date format" 2 "$TODO_BIN" add "Test" --due "2026/08/20"

# Test 9: Invalid command should fail
run_test "reject invalid command" 2 "$TODO_BIN" invalid

# Test 10: Add without text should fail
run_test "reject add without text" 2 "$TODO_BIN" add

# Test 11: Due without ID should fail
run_test "reject due without id" 2 "$TODO_BIN" due

# Test 12: Done without ID should fail
run_test "reject done without id" 2 "$TODO_BIN" done

# Test 13: Add todo with future date
run_test "add todo with future date" 0 "$TODO_BIN" add "Future task" --due "2027-12-31 23:59:59"

# TODO: Test 14: Review command (currently stubbed)
# run_test "review command" 0 "$TODO_BIN" review

# Print summary
echo ""
echo "========== Test Summary =========="
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"
echo "=================================="

if [ "$FAILED" -gt 0 ]; then
    exit 1
fi

exit 0
