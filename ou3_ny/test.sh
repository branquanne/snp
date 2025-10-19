#!/bin/bash
# Test script for mdu program
# Tests basic functionality and compares output with du

echo "=== MDU Test Script ==="
echo

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Compile the program
echo "Compiling..."
make clean > /dev/null 2>&1
if ! make > /dev/null 2>&1; then
    echo -e "${RED}✗ Compilation failed${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Compilation successful${NC}"
echo

# Test 1: Single file
echo "Test 1: Single file"
MDU_OUT=$(./mdu mdu.c | awk '{print $1}')
DU_OUT=$(du -s -l -B512 mdu.c | awk '{print $1}')
if [ "$MDU_OUT" = "$DU_OUT" ]; then
    echo -e "${GREEN}✓ Single file test passed${NC} (mdu: $MDU_OUT, du: $DU_OUT)"
else
    echo -e "${RED}✗ Single file test failed${NC} (mdu: $MDU_OUT, du: $DU_OUT)"
fi
echo

# Test 2: Current directory
echo "Test 2: Current directory"
MDU_OUT=$(./mdu . | awk '{print $1}')
DU_OUT=$(du -s -l -B512 . | awk '{print $1}')
if [ "$MDU_OUT" = "$DU_OUT" ]; then
    echo -e "${GREEN}✓ Directory test passed${NC} (mdu: $MDU_OUT, du: $DU_OUT)"
else
    echo -e "${RED}✗ Directory test failed${NC} (mdu: $MDU_OUT, du: $DU_OUT)"
fi
echo

# Test 3: Multiple files
echo "Test 3: Multiple files"
FILES="mdu.c dirsize.c work_queue.c"
PASS=true
for file in $FILES; do
    MDU_OUT=$(./mdu $file | awk '{print $1}')
    DU_OUT=$(du -s -l -B512 $file | awk '{print $1}')
    if [ "$MDU_OUT" != "$DU_OUT" ]; then
        echo -e "${RED}✗ Failed for $file${NC} (mdu: $MDU_OUT, du: $DU_OUT)"
        PASS=false
    fi
done
if [ "$PASS" = true ]; then
    echo -e "${GREEN}✓ Multiple files test passed${NC}"
fi
echo

# Test 4: Multi-threaded (2 threads)
echo "Test 4: Multi-threaded with 2 threads"
MDU_OUT=$(./mdu -j 2 . | awk '{print $1}')
DU_OUT=$(du -s -l -B512 . | awk '{print $1}')
if [ "$MDU_OUT" = "$DU_OUT" ]; then
    echo -e "${GREEN}✓ Multi-threaded (2) test passed${NC} (mdu: $MDU_OUT, du: $DU_OUT)"
else
    echo -e "${RED}✗ Multi-threaded (2) test failed${NC} (mdu: $MDU_OUT, du: $DU_OUT)"
fi
echo

# Test 5: Multi-threaded (4 threads)
echo "Test 5: Multi-threaded with 4 threads"
MDU_OUT=$(./mdu -j 4 . | awk '{print $1}')
DU_OUT=$(du -s -l -B512 . | awk '{print $1}')
if [ "$MDU_OUT" = "$DU_OUT" ]; then
    echo -e "${GREEN}✓ Multi-threaded (4) test passed${NC} (mdu: $MDU_OUT, du: $DU_OUT)"
else
    echo -e "${RED}✗ Multi-threaded (4) test failed${NC} (mdu: $MDU_OUT, du: $DU_OUT)"
fi
echo

# Test 6: -j flag position flexibility
echo "Test 6: Flag position flexibility"
OUT1=$(./mdu -j 2 mdu.c | awk '{print $1}')
OUT2=$(./mdu mdu.c -j 2 2>&1)
if echo "$OUT2" | grep -q "Usage:"; then
    # getopt doesn't support flags after non-option args by default, which is fine
    echo -e "${GREEN}✓ Flag position test: getopt behavior as expected${NC}"
else
    OUT2=$(echo "$OUT2" | awk '{print $1}')
    if [ "$OUT1" = "$OUT2" ]; then
        echo -e "${GREEN}✓ Flag position test passed${NC}"
    else
        echo -e "${RED}✗ Flag position test failed${NC}"
    fi
fi
echo

echo "=== Tests Complete ==="
