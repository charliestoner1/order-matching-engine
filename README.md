# High-Performance Order Matching Engine

A microsecond-latency order matching engine built with C++ featuring a B-Tree implementation for order book management and real-time web visualization.

## 🚀 Performance Metrics

Our B-Tree implementation achieves industry-grade performance:
- **Order Addition**: 1-5 microseconds ✓
- **Order Matching**: ~27-52 microseconds per trade
- **Query Operations**: < 1 microsecond
- **Throughput**: 15+ orders/second sustained, capable of 200,000+ orders/second

All benchmarks meet the **<10 microsecond requirement** for order operations.

## 📋 Features

- **B-Tree Order Book**: Optimized multi-key node structure for superior cache performance
- **Price-Time Priority**: Fair FIFO matching at each price level
- **Real-Time Visualization**: Live order book updates via web interface
- **Comprehensive Testing**: Unit tests covering all core functionality
- **Performance Benchmarks**: Built-in benchmarking suite

## 🛠️ Technology Stack

- **Core Engine**: C++17
- **Data Structure**: B-Tree (degree 32)
- **Build System**: CMake
- **IDE**: CLion with MSYS2/MinGW64
- **Visualization**: HTML5/JavaScript
- **Platform**: Windows (Surface Studio)

## 📁 Project Structure

```
OrderMatchingEngine/
├── src/
│   ├── main.cpp                    # Main application with order generation
│   ├── core/                       # Core trading components
│   │   ├── Order.h                 # Order structure
│   │   ├── Trade.h                 # Trade structure
│   │   ├── OrderBook.h/cpp         # Order book interface
│   │   └── MatchingEngine.h        # Engine managing multiple order books
│   ├── implementations/
│   │   └── BTreeOrderBook.h/cpp    # B-Tree implementation
│   └── utils/
│       └── Timer.h                 # Performance timing utilities
├── visualization/
│   ├── index.html                  # Real-time order book display
│   └── data/                       # JSON output directory
├── benchmark/
│   └── OrderBookBenchmark.cpp      # Performance benchmarks
├── test/
│   └── test_order_matching.cpp     # Unit tests
├── scripts/
│   ├── run_demo.ps1               # Run full demo with visualization
│   ├── run_tests.ps1              # Run unit tests
│   └── run_benchmarks.ps1         # Run performance benchmarks
└── CMakeLists.txt                 # Build configuration
```

## 🚦 Prerequisites

- **CLion IDE** (2023.1 or later)
- **MSYS2** with MinGW64 toolchain
- **CMake** 3.10+
- **Python** 3.x (for web server)
- **Modern web browser** (Chrome/Firefox/Edge)

## 🔧 Setup & Build

### Using CLion (Recommended)

1. **Open Project**
    - File → Open → Select project root directory
    - CLion will automatically detect CMakeLists.txt

2. **Build Project**
    - Press `Ctrl+F9` or Build → Build Project
    - Select build configuration (Debug/Release)

3. **Build Targets**
    - `ordermatching` - Main application
    - `test_order_matching` - Unit tests
    - `benchmark` - Performance benchmarks

## 🏃 Running the Project

### Demo with Live Visualization

The demo runs the matching engine for 60 seconds, processing ~900 orders with real-time visualization.

**In CLion's Terminal** (View → Tool Windows → Terminal):

```powershell
# Terminal 1: Start web server
cd visualization
python -m http.server 8080

# Terminal 2: Run the demo
.\scripts\run_demo.ps1
```

The browser will open automatically showing:
- Live order book with bid/ask levels
- Real-time performance metrics
- Order throughput and market depth
- B-Tree implementation indicator

### Running Tests

Execute comprehensive unit tests:

```powershell
.\scripts\run_tests.ps1
```

Tests verify:
- ✓ Basic order matching
- ✓ Price priority
- ✓ Time priority
- ✓ Order cancellation
- ✓ Market data queries
- ✓ Stress test (10,000 orders)

### Running Benchmarks

Measure performance metrics:

```powershell
.\scripts\run_benchmarks.ps1
```

Benchmarks test:
- Order insertion (warm and cold)
- Order insertion with 100k existing orders
- Order matching performance
- Query operation latency

## 📊 Performance Results

### Benchmark Output Example
```
=== Benchmark: Add Order (Warmed Up) ===
Time to add order: 1.00 microseconds
Requirement: <10 microseconds
Status: PASS

=== Benchmark: Add Order (Full Book - 100k orders) ===
Time to add order to full book: 5.00 microseconds
Requirement: <10 microseconds
Status: PASS
```

### Live Demo Metrics
- **Total Orders**: ~900 in 60 seconds
- **Orders/Second**: ~15 sustained
- **Active Orders**: 400-900 concurrent
- **Market Depth**: 20 price levels
- **Update Rate**: 5-6 updates/second to visualization

## 🔍 Key Implementation Details

### B-Tree Order Book
- **Degree**: 32 (optimized for cache line size)
- **Node Capacity**: 63 keys max (2*degree - 1)
- **Leaf Chaining**: Linked list for efficient traversal
- **Binary Search**: O(log n) price lookups within nodes

### Order Matching Algorithm
```cpp
while (best_bid >= best_ask) {
    // Match orders at crossing prices
    // Remove filled orders
    // Generate trades
}
```

### Visualization Integration
- C++ engine writes JSON to `visualization/data/`
- Web interface polls every 200ms
- Updates display price levels, spread, and metrics

## 🤝 Team Contributions

This B-Tree implementation was developed as part of a team project comparing different order book data structures for high-frequency trading systems.

## 📝 Notes

- The engine uses simulated market data with random order generation
- Price movements follow a random walk pattern with realistic spread maintenance
- All times are in microseconds (μs) for precision measurement

## 🐛 Troubleshooting

### "ordermatching.exe not found"
- Build the project first with `Ctrl+F9` in CLion

### "Python not found"
- Install Python and ensure it's in PATH

### No data in visualization
- Check that web server is running from `visualization` directory
- Ensure C++ engine is running and writing to correct path

### Performance issues
- Use Release build for benchmarks
- Close unnecessary applications
- Ensure no antivirus is scanning the data directory