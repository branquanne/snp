#!/bin/bash
# Valgrind test script for mdu
# Checks for memory leaks and thread safety issues

echo "=== MDU Valgrind Tests ==="
echo

# Ensure program is compiled with -g flag
if [ ! -f "./mdu" ]; then
    echo "Compiling mdu with debug symbols..."
    make clean > /dev/null 2>&1
    make > /dev/null 2>&1
fi

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "Test 1: Memory leak check (single-threaded)"
echo "Command: valgrind --leak-check=full --show-leak-kinds=all ./mdu ."
valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./mdu . > /dev/null 2> valgrind_memcheck.log
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ No memory leaks detected (single-threaded)${NC}"
else
    echo -e "${RED}✗ Memory leaks detected (single-threaded)${NC}"
    echo "See valgrind_memcheck.log for details"
fi
echo

echo "Test 2: Memory leak check (multi-threaded)"
echo "Command: valgrind --leak-check=full --show-leak-kinds=all ./mdu -j 4 ."
valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./mdu -j 4 . > /dev/null 2> valgrind_memcheck_mt.log
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ No memory leaks detected (multi-threaded)${NC}"
else
    echo -e "${RED}✗ Memory leaks detected (multi-threaded)${NC}"
    echo "See valgrind_memcheck_mt.log for details"
fi
echo

echo "Test 3: Thread safety check (helgrind)"
echo "Command: valgrind --tool=helgrind ./mdu -j 4 ."
echo -e "${YELLOW}Note: This may take a while...${NC}"
valgrind --tool=helgrind ./mdu -j 4 . > /dev/null 2> valgrind_helgrind.log
if grep -q "ERROR SUMMARY: 0 errors" valgrind_helgrind.log; then
    echo -e "${GREEN}✓ No thread safety issues detected${NC}"
else
    ERROR_COUNT=$(grep "ERROR SUMMARY:" valgrind_helgrind.log | awk '{print $4}')
    if [ "$ERROR_COUNT" = "0" ]; then
        echo -e "${GREEN}✓ No thread safety issues detected${NC}"
    else
        echo -e "${RED}✗ Thread safety issues detected${NC}"
        echo "See valgrind_helgrind.log for details"
    fi
fi
echo

echo "Test 4: File descriptor leak check"
echo "Command: valgrind --track-fds=yes ./mdu -j 2 ."
valgrind --track-fds=yes --error-exitcode=1 ./mdu -j 2 . > /dev/null 2> valgrind_fds.log
if grep -q "FILE DESCRIPTORS: 3 open" valgrind_fds.log; then
    echo -e "${GREEN}✓ No file descriptor leaks detected${NC}"
else
    FD_COUNT=$(grep "FILE DESCRIPTORS:" valgrind_fds.log | awk '{print $3}')
    if [ "$FD_COUNT" = "3" ]; then
        echo -e "${GREEN}✓ No file descriptor leaks detected${NC}"
    else
        echo -e "${YELLOW}⚠ Possible file descriptor leak (open: $FD_COUNT, expected: 3)${NC}"
        echo "See valgrind_fds.log for details"
    fi
fi
echo

echo "=== Valgrind Tests Complete ==="
echo "Log files created:"
echo "  - valgrind_memcheck.log (memory leaks, single-threaded)"
echo "  - valgrind_memcheck_mt.log (memory leaks, multi-threaded)"
echo "  - valgrind_helgrind.log (thread safety)"
echo "  - valgrind_fds.log (file descriptor leaks)"
