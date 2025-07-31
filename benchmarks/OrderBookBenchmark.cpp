#include "../src/utils/Timer.h"
#include "../src/implementations/BTreeOrderBook.h"
#include "../src/core/Order.h"

#include <iostream>
#include <memory>
#include <iomanip>

using namespace order_matching;
using namespace order_matching::utils;

void benchmark_add_order_warmedup_book() {
    std::cout << "\n=== Benchmark: Add Order (Warmed Up) ===" << std::endl;

    BTreeOrderBook book("AAPL");

    // Warm-up with 1000 orders
    for (int i = 0; i < 1000; ++i) {
        auto order = std::make_shared<Order>(i, BUY, 100.0 + (i % 100) * 0.01, 10, "AAPL");
        book.add_order(order);
    }

    // Measure adding one more order
    auto order = std::make_shared<Order>(1000, BUY, 105.0, 10, "AAPL");
    Timer timer;
    book.add_order(order);
    double elapsed = timer.elapsed_microseconds();

    std::cout << "Time to add order: " << elapsed << " microseconds" << std::endl;
    std::cout << "Requirement: <10 microseconds" << std::endl;
    std::cout << "Status: " << (elapsed < 10 ? "PASS" : "FAIL") << std::endl;
}

void benchmark_add_order_full_book() {
    std::cout << "\n=== Benchmark: Add Order (Full Book - 100k orders) ===" << std::endl;

    BTreeOrderBook book("AAPL");

    // Fill with 100,000 orders at different price levels
    std::cout << "Building order book with 100,000 orders..." << std::endl;
    for (int i = 0; i < 100000; ++i) {
        double price = 100.0 + (i % 1000) * 0.01;  // 1000 different price levels
        auto order = std::make_shared<Order>(i, BUY, price, 10, "AAPL");
        book.add_order(order);

        if ((i + 1) % 10000 == 0) {
            std::cout << "  Added " << (i + 1) << " orders..." << std::endl;
        }
    }

    // Measure adding one more order
    auto order = std::make_shared<Order>(100000, BUY, 105.0, 10, "AAPL");
    Timer timer;
    book.add_order(order);
    double elapsed = timer.elapsed_microseconds();

    std::cout << "Time to add order to full book: " << elapsed << " microseconds" << std::endl;
    std::cout << "Requirement: <10 microseconds" << std::endl;
    std::cout << "Status: " << (elapsed < 10 ? "PASS" : "FAIL") << std::endl;
}

void benchmark_match_orders() {
    std::cout << "\n=== Benchmark: Match Orders ===" << std::endl;

    BTreeOrderBook book("AAPL");

    // Add buy orders
    for (int i = 0; i < 100; ++i) {
        auto order = std::make_shared<Order>(i, BUY, 100.0 - i * 0.01, 10, "AAPL");
        book.add_order(order);
    }

    // Add sell orders
    for (int i = 100; i < 200; ++i) {
        auto order = std::make_shared<Order>(i, SELL, 100.0 + (i - 100) * 0.01, 10, "AAPL");
        book.add_order(order);
    }

    std::cout << "Order book state before matching:" << std::endl;
    std::cout << "  Best Bid: $" << book.get_best_bid() << std::endl;
    std::cout << "  Best Ask: $" << book.get_best_ask() << std::endl;

    // Measure matching
    Timer timer;
    std::vector<Trade> trades = book.match_orders();
    double elapsed = timer.elapsed_microseconds();

    std::cout << "Matched " << trades.size() << " trades in " << elapsed << " microseconds" << std::endl;
    if (!trades.empty()) {
        std::cout << "Average time per trade: " << (elapsed / trades.size()) << " microseconds" << std::endl;
    }
}

void benchmark_query_operations() {
    std::cout << "\n=== Benchmark: Query Operations ===" << std::endl;

    BTreeOrderBook book("AAPL");

    // Fill book
    for (int i = 0; i < 10000; ++i) {
        double price = 100.0 + (i % 100) * 0.01;
        Side side = (i % 2 == 0) ? BUY : SELL;
        auto order = std::make_shared<Order>(i, side, price, 10, "AAPL");
        book.add_order(order);
    }

    // Benchmark get_best_bid
    Timer timer1;
    double bid = book.get_best_bid();
    double time1 = timer1.elapsed_microseconds();
    std::cout << "get_best_bid(): " << time1 << " microseconds (result: $" << bid << ")" << std::endl;

    // Benchmark get_best_ask
    Timer timer2;
    double ask = book.get_best_ask();
    double time2 = timer2.elapsed_microseconds();
    std::cout << "get_best_ask(): " << time2 << " microseconds (result: $" << ask << ")" << std::endl;

    // Benchmark get_bid_levels
    Timer timer3;
    auto levels = book.get_bid_levels(10);
    double time3 = timer3.elapsed_microseconds();
    std::cout << "get_bid_levels(10): " << time3 << " microseconds (returned " << levels.size() << " levels)" << std::endl;
}

int main() {
    std::cout << "B-Tree Order Book - Performance Benchmarks" << std::endl;
    std::cout << "=========================================" << std::endl;

    // Set precision for output
    std::cout << std::fixed << std::setprecision(2);

    // Run benchmarks
    benchmark_add_order_warmedup_book();
    benchmark_add_order_full_book();
    benchmark_match_orders();
    benchmark_query_operations();

    std::cout << "\nBenchmarks complete!" << std::endl;

    return 0;
}