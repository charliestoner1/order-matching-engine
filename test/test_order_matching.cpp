#include <iostream>
#include <cassert>
#include <random>
#include <iomanip>
#include "../src/core/MatchingEngine.h"
#include "../src/implementations/BTreeOrderBook.h"
#include "../src/utils/Timer.h"

using namespace order_matching;
using namespace order_matching::utils;

class OrderMatchingTester {
private:
    std::mt19937 rng;
    std::uniform_real_distribution<double> price_dist;
    std::uniform_real_distribution<double> qty_dist;
    std::uniform_int_distribution<int> side_dist;

public:
    OrderMatchingTester()
        : rng(std::random_device{}()),
          price_dist(90.0, 110.0),
          qty_dist(1.0, 1000.0),
          side_dist(0, 1) {}

    void test_basic_matching() {
        std::cout << "\n=== Test: Basic Order Matching ===" << std::endl;

        BTreeOrderBook book("AAPL");

        // Add buy order
        auto buy_order = std::make_shared<Order>(1, BUY, 100.0, 100, "AAPL");
        assert(book.add_order(buy_order));

        // Add sell order at same price
        auto sell_order = std::make_shared<Order>(2, SELL, 100.0, 50, "AAPL");
        assert(book.add_order(sell_order));

        // Match orders
        auto trades = book.match_orders();
        assert(trades.size() == 1);
        assert(trades[0].get_price() == 100.0);
        assert(trades[0].get_quantity() == 50.0);
        assert(trades[0].get_buy_order_id() == 1);
        assert(trades[0].get_sell_order_id() == 2);

        // Check remaining quantities
        assert(buy_order->get_remaining_quantity() == 50.0);
        assert(sell_order->get_remaining_quantity() == 0.0);
        assert(sell_order->is_filled());

        std::cout << "✓ Basic matching test passed" << std::endl;
    }

    void test_price_priority() {
        std::cout << "\n=== Test: Price Priority ===" << std::endl;

        BTreeOrderBook book("AAPL");

        // Add multiple buy orders at different prices
        book.add_order(std::make_shared<Order>(1, BUY, 99.0, 100, "AAPL"));
        book.add_order(std::make_shared<Order>(2, BUY, 100.0, 100, "AAPL"));
        book.add_order(std::make_shared<Order>(3, BUY, 98.0, 100, "AAPL"));

        // Add sell order
        book.add_order(std::make_shared<Order>(4, SELL, 99.0, 100, "AAPL"));

        // Match orders - should match with highest buy price (100.0)
        auto trades = book.match_orders();
        assert(trades.size() == 1);
        assert(trades[0].get_buy_order_id() == 2);  // Highest price buy order

        std::cout << "✓ Price priority test passed" << std::endl;
    }

    void test_time_priority() {
        std::cout << "\n=== Test: Time Priority ===" << std::endl;

        BTreeOrderBook book("AAPL");

        // Add multiple buy orders at same price
        book.add_order(std::make_shared<Order>(1, BUY, 100.0, 50, "AAPL"));
        book.add_order(std::make_shared<Order>(2, BUY, 100.0, 50, "AAPL"));
        book.add_order(std::make_shared<Order>(3, BUY, 100.0, 50, "AAPL"));

        // Add sell order
        book.add_order(std::make_shared<Order>(4, SELL, 100.0, 50, "AAPL"));

        // Match orders - should match with first buy order
        auto trades = book.match_orders();
        assert(trades.size() == 1);
        assert(trades[0].get_buy_order_id() == 1);  // First in time

        std::cout << "✓ Time priority test passed" << std::endl;
    }

    void test_order_cancellation() {
        std::cout << "\n=== Test: Order Cancellation ===" << std::endl;

        BTreeOrderBook book("AAPL");

        // Add orders
        book.add_order(std::make_shared<Order>(1, BUY, 100.0, 100, "AAPL"));
        book.add_order(std::make_shared<Order>(2, BUY, 101.0, 100, "AAPL"));

        // Cancel order
        assert(book.cancel_order(1));
        assert(!book.cancel_order(1));  // Double cancel should fail
        assert(!book.cancel_order(999)); // Non-existent order

        // Check best bid is now 101.0
        assert(book.get_best_bid() == 101.0);

        std::cout << "✓ Order cancellation test passed" << std::endl;
    }

    void test_market_data_queries() {
        std::cout << "\n=== Test: Market Data Queries ===" << std::endl;

        BTreeOrderBook book("AAPL");

        // Empty book
        assert(book.get_best_bid() == 0.0);
        assert(book.get_best_ask() == 0.0);
        assert(book.get_total_orders() == 0);

        // Add orders
        book.add_order(std::make_shared<Order>(1, BUY, 99.0, 100, "AAPL"));
        book.add_order(std::make_shared<Order>(2, BUY, 100.0, 200, "AAPL"));
        book.add_order(std::make_shared<Order>(3, SELL, 101.0, 150, "AAPL"));
        book.add_order(std::make_shared<Order>(4, SELL, 102.0, 250, "AAPL"));

        assert(book.get_best_bid() == 100.0);
        assert(book.get_best_ask() == 101.0);
        assert(book.get_bid_count() == 2);
        assert(book.get_ask_count() == 2);
        assert(book.get_total_orders() == 4);

        // Get levels
        auto bid_levels = book.get_bid_levels(10);
        assert(bid_levels.size() == 2);
        assert(bid_levels[0].price == 100.0);
        assert(bid_levels[0].quantity == 200.0);
        assert(bid_levels[1].price == 99.0);
        assert(bid_levels[1].quantity == 100.0);

        std::cout << "✓ Market data queries test passed" << std::endl;
    }

    void test_stress_random_orders() {
        std::cout << "\n=== Test: Stress Test with Random Orders ===" << std::endl;

        BTreeOrderBook book("AAPL");
        const int num_orders = 10000;

        Timer timer;

        // Add random orders
        for (int i = 0; i < num_orders; ++i) {
            Side side = side_dist(rng) == 0 ? BUY : SELL;
            double price = price_dist(rng);
            double qty = qty_dist(rng);

            auto order = std::make_shared<Order>(i, side, price, qty, "AAPL");
            book.add_order(order);
        }

        double add_time = timer.elapsed_milliseconds();
        std::cout << "Added " << num_orders << " orders in " << add_time << " ms" << std::endl;
        std::cout << "Average: " << (add_time * 1000 / num_orders) << " μs per order" << std::endl;

        // Match orders
        timer.start();
        auto trades = book.match_orders();
        double match_time = timer.elapsed_milliseconds();

        std::cout << "Matched " << trades.size() << " trades in " << match_time << " ms" << std::endl;
        if (!trades.empty()) {
            std::cout << "Average: " << (match_time * 1000 / trades.size()) << " μs per trade" << std::endl;
        }

        std::cout << "✓ Stress test completed" << std::endl;
    }

    void run_all_tests() {
        std::cout << "Running Order Matching Engine Tests" << std::endl;
        std::cout << "===================================" << std::endl;

        test_basic_matching();
        test_price_priority();
        test_time_priority();
        test_order_cancellation();
        test_market_data_queries();
        test_stress_random_orders();

        std::cout << "\n✓ All tests passed!" << std::endl;
    }
};

void test_matching_engine() {
    std::cout << "\n=== Test: Matching Engine Integration ===" << std::endl;

    MatchingEngine engine;

    // Create order books for multiple symbols
    engine.create_order_book("AAPL", std::make_unique<BTreeOrderBook>("AAPL"));
    engine.create_order_book("GOOGL", std::make_unique<BTreeOrderBook>("GOOGL"));

    // Test AAPL orders
    engine.submit_order(std::make_shared<Order>(1, BUY, 150.0, 100, "AAPL"));
    engine.submit_order(std::make_shared<Order>(2, SELL, 150.0, 50, "AAPL"));

    auto aapl_trades = engine.match_orders("AAPL");
    assert(aapl_trades.size() == 1);

    // Test GOOGL orders
    engine.submit_order(std::make_shared<Order>(3, BUY, 2800.0, 10, "GOOGL"));
    engine.submit_order(std::make_shared<Order>(4, SELL, 2799.0, 10, "GOOGL"));

    auto googl_trades = engine.match_orders("GOOGL");
    assert(googl_trades.size() == 1);

    // Test invalid symbol
    auto invalid_order = std::make_shared<Order>(5, BUY, 100.0, 10, "TSLA");
    assert(!engine.submit_order(invalid_order));

    std::cout << "✓ Matching engine integration test passed" << std::endl;
}

int main() {
    try {
        OrderMatchingTester tester;
        tester.run_all_tests();

        test_matching_engine();

        std::cout << "\n========================================" << std::endl;
        std::cout << "All tests completed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}