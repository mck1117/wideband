#!/bin/bash
# Format firmware code with clang-format, excluding external code
# Usage: ./format.sh [check]
#   format.sh              - Format files in-place
#   format.sh check        - Check if formatting is needed (exit 1 if changes needed)

set -e

CHECK_ONLY=false
if [ "$1" = "check" ]; then
    CHECK_ONLY=true
fi

# Find all C/C++ files in firmware and test directories
files=$(find firmware test for_rusefi \
    -path "firmware/ChibiOS" -prune -o \
    -path "firmware/ext" -prune -o \
    -path "firmware/cmsis-svd" -prune -o \
    -path "firmware/libfirmware" -prune -o \
    -path "*cfg*" -prune -o \
    -path "firmware/boards/f1*" -prune -o \
    -path "test/googletest" -prune -o \
    -path "for_rusefi/wideband_image.h" -prune -o \
    -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.cc" \) \
    -print)

if [ -z "$files" ]; then
    echo "No C/C++ files found"
    exit 0
fi

if [ "$CHECK_ONLY" = true ]; then
    # Check mode: show formatting diffs
    echo "Checking code formatting..."

    has_changes=false
    while IFS= read -r file; do
        echo "Checking $file..."

        # Get the formatted version
        formatted=$(./clang-format "$file")

        # Compare with original
        if ! diff -u "$file" <(echo "$formatted") > /dev/null 2>&1; then
            # Changes needed - show the diff
            echo ""
            echo "=== Formatting changes needed in: $file ==="
            diff -u "$file" <(echo "$formatted") || true
            has_changes=true
        fi
    done <<< "$files"

    if [ "$has_changes" = true ]; then
        echo ""
        echo "Formatting check failed. Run './format.sh' to fix."
        exit 1
    else
        echo "Code formatting is correct."
        exit 0
    fi
else
    # Format mode: apply changes in-place
    echo "Formatting code..."
    echo "$files" | xargs -d '\n' ./clang-format -i
    echo "Formatting complete"
    exit 0
fi
