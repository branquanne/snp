# mdu - Disk Usage Calculator

## Description
`mdu` is a disk usage calculator similar to the Unix `du` command. It calculates the total disk space used by files and directories, with support for parallel processing using multiple threads.

## Features
- Single-threaded recursive directory traversal
- Multi-threaded parallel directory traversal for improved performance
- Output format matching `du -s -l -B512`
- Thread-safe work queue implementation
- Proper error handling for all system calls
- No memory leaks or file descriptor leaks

## Compilation
```bash
make
```

To clean build artifacts:
```bash
make clean
```

## Usage
```bash
mdu [-j antal_trådar] fil ...
```

### Options
- `-j antal_trådar`: Number of threads to use for parallel processing (default: 1)

### Examples
```bash
# Single-threaded mode
./mdu /path/to/directory

# Multi-threaded mode with 4 threads
./mdu -j 4 /path/to/directory

# Process multiple files/directories
./mdu -j 2 file1.txt dir1 dir2
```

## Implementation Details

### Files
- **mdu.c**: Main program file with command-line parsing
- **dirsize.c**: Directory size calculation (single and multi-threaded)
- **dirsize.h**: Header file for directory size functions
- **work_queue.c**: Thread-safe work queue implementation
- **work_queue.h**: Header file for work queue
- **Makefile**: Build configuration

### Key Features
1. **Circular Buffer Queue**: Efficient O(1) push/pop operations
2. **Condition Variables**: Proper thread synchronization without busy waiting
3. **Error Handling**: All system calls checked with appropriate error messages
4. **Memory Management**: No memory leaks, all resources properly freed

## Output Format
The program outputs the size in 512-byte blocks followed by the filename/directory name, separated by a tab character (matching `du -s -l -B512`).

## Testing
Run valgrind to check for memory leaks:
```bash
valgrind --leak-check=full ./mdu .
```

Run helgrind to check for thread safety issues:
```bash
valgrind --tool=helgrind ./mdu -j 4 .
```

## Requirements Met
- ✅ Uses getopt for argument parsing
- ✅ Uses lstat, opendir, readdir, closedir
- ✅ Uses pthread_create and pthread_join
- ✅ Proper error handling with perror/strerror
- ✅ No global variables
- ✅ No memory leaks or file descriptor leaks
- ✅ Thread-safe implementation
- ✅ Compiles without warnings with strict flags
- ✅ Separate compilation with proper Makefile
