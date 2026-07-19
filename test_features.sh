#!/bin/bash

PROJ=~/C-plus-learn/SecurityGuardManagement/src/ui

echo "============================================"
echo "  Security Guard Manager - Feature Check"
echo "============================================"
echo ""

check_file() {
    local file=$1
    local name=$2
    if [ -f "$file" ]; then
        local lines=$(wc -l < "$file")
        if [ "$lines" -gt 50 ]; then
            echo "[OK]   $name ($lines lines)"
        else
            echo "[WARN] $name ($lines lines - might be incomplete)"
        fi
    else
        echo "[MISS] $name (file not found)"
    fi
}

echo "--- Module Files ---"
check_file "$PROJ/GuardWidget.cpp" "GuardWidget"
check_file "$PROJ/GuardDialog.cpp" "GuardDialog"
check_file "$PROJ/ClientWidget.cpp" "ClientWidget"
check_file "$PROJ/ClientDialog.cpp" "ClientDialog"
check_file "$PROJ/SiteWidget.cpp" "SiteWidget"
check_file "$PROJ/SiteDialog.cpp" "SiteDialog"
check_file "$PROJ/AttendanceWidget.cpp" "AttendanceWidget"
check_file "$PROJ/DutyWidget.cpp" "DutyWidget"
check_file "$PROJ/DutyDialog.cpp" "DutyDialog"
echo ""

echo "--- Button Checks ---"
for feature in "Add Guard" "Edit" "Delete" "Add Client" "Add Site" "Save Attendance" "Mark All Present" "Export CSV"; do
    found=$(grep -rl "$feature" "$PROJ/"*.cpp 2>/dev/null | wc -l)
    if [ "$found" -gt 0 ]; then
        echo "[OK]   '$feature' found in $found file(s)"
    else
        echo "[MISS] '$feature' not found in any file"
    fi
done
echo ""

echo "--- Signal/Slot Checks ---"
for signal in "navigateToGuard" "navigateToAttendance" "navigateToSalary" "navigateToReports" "navigateToBackup"; do
    found=$(grep -rl "$signal" "$PROJ/"*.cpp "$PROJ/"*.h 2>/dev/null | wc -l)
    if [ "$found" -gt 0 ]; then
        echo "[OK]   Signal '$signal' connected in $found file(s)"
    else
        echo "[MISS] Signal '$signal' not found"
    fi
done
echo ""

echo "--- Database Table Checks ---"
DB_FILE=~/C-plus-learn/SecurityGuardManagement/build/database.db
if [ -f "$DB_FILE" ]; then
    echo "[OK]   Database file exists"
    echo ""
    echo "Tables in database:"
    sqlite3 "$DB_FILE" ".tables"
    echo ""
    echo "Record counts:"
    for table in Guards Clients Sites Attendance Duty Salary Leave Incidents; do
        count=$(sqlite3 "$DB_FILE" "SELECT COUNT(*) FROM $table;" 2>/dev/null)
        if [ -n "$count" ]; then
            echo "  $table: $count records"
        fi
    done
    echo ""
    echo "Admin user:"
    sqlite3 "$DB_FILE" "SELECT id, username, role FROM Users;"
else
    echo "[WARN] Database not found - run the app once first"
fi
echo ""

echo "--- Build Binary Check ---"
BIN=~/C-plus-learn/SecurityGuardManagement/build/SecurityGuardManager
if [ -f "$BIN" ]; then
    echo "[OK]   Binary exists ($(du -h "$BIN" | cut -f1))"
    echo "       Last modified: $(stat -c '%y' "$BIN")"
else
    echo "[MISS] Binary not found - rebuild needed"
fi
echo ""

echo "--- CMakeLists.txt Source List ---"
echo "Files listed in CMakeLists.txt:"
grep "\.cpp" ~/C-plus-learn/SecurityGuardManagement/CMakeLists.txt | sed 's/^/  /'
echo ""

echo "============================================"
echo "  Check Complete"
echo "============================================"
