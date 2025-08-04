#!/bin/bash

# Benchmark runner script for Order Matching Engine

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_status() {
    echo -e "${GREEN}[BENCH]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if benchmark executable exists
BENCHMARK_EXEC=""
if [ -f "cmake-build-release/benchmark" ]; then
    BENCHMARK_EXEC="cmake-build-release/benchmark"
elif [ -f "cmake-build-debug/benchmark" ]; then
    BENCHMARK_EXEC="cmake-build-debug/benchmark"
elif [ -f "build/benchmark" ]; then
    BENCHMARK_EXEC="build/benchmark"
else
    print_error "Benchmark executable not found. Please build the project first."
    echo "Run: ./build.sh"
    exit 1
fi

print_header "Order Matching Engine - Performance Benchmarks"
print_status "Using benchmark executable: $BENCHMARK_EXEC"
echo ""

# Set CPU affinity for consistent results (Linux only)
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if command -v taskset &> /dev/null; then
        print_status "Setting CPU affinity to core 0 for consistent results..."
        TASKSET_CMD="taskset -c 0"
    fi
fi

# Disable CPU frequency scaling for benchmarks (Linux only)
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if [ -w /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor ]; then
        print_status "Setting CPU governor to performance mode..."
        for i in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
            echo "performance" > $i 2>/dev/null
        done
    else
        print_status "Cannot set CPU governor (need root). Results may vary."
    fi
fi

# Run benchmarks multiple times for consistency
NUM_RUNS=3
RESULTS_FILE="benchmark_results_$(date +%Y%m%d_%H%M%S).txt"

print_status "Running benchmarks $NUM_RUNS times..."
print_status "Results will be saved to: $RESULTS_FILE"
echo ""

echo "Benchmark Results - $(date)" > $RESULTS_FILE
echo "======================================" >> $RESULTS_FILE

for i in $(seq 1 $NUM_RUNS); do
    print_status "Run $i of $NUM_RUNS"
    echo -e "\n--- Run $i ---" >> $RESULTS_FILE

    if [ -n "$TASKSET_CMD" ]; then
        $TASKSET_CMD $BENCHMARK_EXEC >> $RESULTS_FILE 2>&1
    else
        $BENCHMARK_EXEC >> $RESULTS_FILE 2>&1
    fi

    if [ $i -lt $NUM_RUNS ]; then
        sleep 2  # Brief pause between runs
    fi
done

echo ""
print_header "Benchmark Summary"

# Extract and display key metrics
print_status "Performance metrics from latest run:"
echo ""

# Extract add order performance
ADD_ORDER_TIME=$(grep -A1 "Add Order (Warmed Up)" $RESULTS_FILE | tail -n1 | grep -oE '[0-9]+\.[0-9]+' | head -n1)
if [ -n "$ADD_ORDER_TIME" ]; then
    echo -e "Add Order (Warmed Up): ${GREEN}${ADD_ORDER_TIME}${NC} microseconds"
    if (( $(echo "$ADD_ORDER_TIME < 10" | bc -l) )); then
        echo -e "                       ${GREEN}✓ PASS${NC} (< 10 μs requirement)"
    else
        echo -e "                       ${RED}✗ FAIL${NC} (< 10 μs requirement)"
    fi
fi

# Extract full book performance
FULL_BOOK_TIME=$(grep -A1 "Add Order (Full Book" $RESULTS_FILE | tail -n1 | grep -oE '[0-9]+\.[0-9]+' | head -n1)
if [ -n "$FULL_BOOK_TIME" ]; then
    echo -e "Add Order (100k book): ${GREEN}${FULL_BOOK_TIME}${NC} microseconds"
    if (( $(echo "$FULL_BOOK_TIME < 10" | bc -l) )); then
        echo -e "                       ${GREEN}✓ PASS${NC} (< 10 μs requirement)"
    else
        echo -e "                       ${RED}✗ FAIL${NC} (< 10 μs requirement)"
    fi
fi

echo ""
print_status "Full results saved to: $RESULTS_FILE"

# Restore CPU governor if we changed it
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if [ -w /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor ]; then
        for i in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
            echo "ondemand" > $i 2>/dev/null
        done
    fi
fi