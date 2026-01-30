#!/bin/bash

# ================= CONFIGURATION =================
# Path to your shell executable (adjust if needed)
# This assumes the script is in 'test-cases' and mysh is in 'project/src'
SHELL_PATH="../src/mysh"

# Directory containing the tests (current directory by default)
TEST_DIR="."
# =================================================

if [ ! -f "$SHELL_PATH" ]; then
    echo "Error: Could not find mysh at $SHELL_PATH"
    echo "Please compile your project first or update SHELL_PATH."
    exit 1
fi

echo "=========================================="
echo "Running tests using shell: $SHELL_PATH"
echo "=========================================="

# Loop through ALL .txt files
for testfile in *.txt; do
    
    # 1. SKIP files that are result keys (ending in _result.txt)
    if [[ "$testfile" == *"_result.txt" ]]; then
        continue
    fi

    filename=$(basename -- "$testfile")
    basename="${filename%.*}"
    
    # 2. MATCH your specific result naming convention (_result.txt)
    expected="${TEST_DIR}/${basename}_result.txt"
    output="${TEST_DIR}/${basename}_output.txt"

    if [ ! -f "$expected" ]; then
        echo "[SKIP] $filename - No result file found ($expected)"
        continue
    fi

    # Run the shell
    $SHELL_PATH < "$testfile" > "$output"

    # Compare output
    if diff -w -i "$expected" "$output" > /dev/null; then
        echo -e "\033[0;32m[PASS]\033[0m $filename"
        rm "$output"
    else
        echo -e "\033[0;31m[FAIL]\033[0m $filename"
        echo "      Check diff: diff -w -i $expected $output"
    fi

done
echo "=========================================="
echo "Testing Complete."
EOF
