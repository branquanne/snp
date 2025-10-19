#!/bin/bash
# Performance benchmark script for mdu
# Compares single-threaded vs multi-threaded performance

echo "=== MDU Performance Benchmark ==="
echo

# Check if a test directory is provided
if [ $# -eq 0 ]; then
    TEST_DIR="."
else
    TEST_DIR="$1"
fi

echo "Testing directory: $TEST_DIR"
echo

# Ensure program is compiled
if [ ! -f "./mdu" ]; then
    echo "Compiling mdu..."
    make > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "ERROR: Compilation failed"
        exit 1
    fi
fi

# Function to run benchmark
run_benchmark() {
    local threads=$1
    local runs=5
    local total=0
    
    for i in $(seq 1 $runs); do
        if [ $threads -eq 1 ]; then
            start=$(date +%s%N)
            ./mdu "$TEST_DIR" > /dev/null
            end=$(date +%s%N)
        else
            start=$(date +%s%N)
            ./mdu -j $threads "$TEST_DIR" > /dev/null
            end=$(date +%s%N)
        fi
        elapsed=$((end - start))
        total=$((total + elapsed))
    done
    
    # Calculate average in milliseconds
    avg=$((total / runs / 1000000))
    echo $avg
}

# Run benchmarks
echo "Running benchmarks (5 runs each)..."
echo

TIME_1=$(run_benchmark 1)
echo "Single-threaded (1 thread):  ${TIME_1}ms"

TIME_2=$(run_benchmark 2)
echo "Multi-threaded  (2 threads): ${TIME_2}ms"
SPEEDUP_2=$(echo "scale=2; $TIME_1 / $TIME_2" | bc)
echo "  Speedup: ${SPEEDUP_2}x"
echo

TIME_4=$(run_benchmark 4)
echo "Multi-threaded  (4 threads): ${TIME_4}ms"
SPEEDUP_4=$(echo "scale=2; $TIME_1 / $TIME_4" | bc)
echo "  Speedup: ${SPEEDUP_4}x"
echo

TIME_8=$(run_benchmark 8)
echo "Multi-threaded  (8 threads): ${TIME_8}ms"
SPEEDUP_8=$(echo "scale=2; $TIME_1 / $TIME_8" | bc)
echo "  Speedup: ${SPEEDUP_8}x"
echo

# Check if requirement is met (at least 2x speedup with multiple threads)
if (( $(echo "$SPEEDUP_2 >= 2.0" | bc -l) )) || (( $(echo "$SPEEDUP_4 >= 2.0" | bc -l) )); then
    echo "✓ Performance requirement met: At least 2x speedup achieved"
else
    echo "⚠ Performance requirement NOT met: No 2x speedup achieved"
    echo "  (Note: This might be due to small test directory or system load)"
fi
