#!/bin/bash

EXECUTABLE="./mdu"

DIRECTORY="../../"

OUTPUT_FILE="performance_results.txt"

echo "" > $OUTPUT_FILE

echo "Running warmup..."
for THREADS in {1..100}; do

    { time $EXECUTABLE -j $THREADS $DIRECTORY; } > /dev/null 2>&1
done

for THREADS in {1..100}; do
    echo "Running with $THREADS threads..."

    { time $EXECUTABLE -j $THREADS $DIRECTORY; } 2>&1 | grep real | awk '{print $2}' | awk -F'm' '{print $2}' | awk -F's' '{printf "%.4f\n", $1}'>> $OUTPUT_FILE
done
echo "Done"