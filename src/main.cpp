#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <random>
#include <filesystem>
#include <iomanip>
#include <sstream>

#include "core/MatchingEngine.h"
#include "implementations/BTreeOrderBook.h"

namespace fs = std::filesystem;
using namespace order_matching;

// write order book data to JSON
void writeOrderBookToJson(MatchingEngine& engine, const std::string& symbol) {
    // Get the order book
    auto* book = dynamic_cast<BTreeOrderBook*>(engine.get_order_book(symbol));
    if (!book) return;

    // Build JSON string
    std::stringstream json;
    json << std::fixed << std::setprecision(2);
    json << "{\n";

    // Basic info
    json << "  \"symbol\": \"" << symbol << "\",\n";
    json << "  \"bestBid\": " << engine.get_best_bid(symbol) << ",\n";
    json << "  \"bestAsk\": " << engine.get_best_ask(symbol) << ",\n";

    // Get bid levels (top 10)
    json << "  \"bids\": [\n";
    auto bids = book->get_bid_levels(10);
    for (size_t i = 0; i < bids.size(); i++) {
        json << "    {\"price\": " << bids[i].price
             << ", \"quantity\": " << bids[i].quantity << "}";
        if (i < bids.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ],\n";

    // Get ask levels (top 10)
    json << "  \"asks\": [\n";
    auto asks = book->get_ask_levels(10);
    for (size_t i = 0; i < asks.size(); i++) {
        json << "    {\"price\": " << asks[i].price
             << ", \"quantity\": " << asks[i].quantity << "}";
        if (i < asks.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ],\n";

    // Add stats for visualization
    json << "  \"stats\": {\n";
    json << "    \"totalOrders\": " << book->get_total_orders() << ",\n";
    json << "    \"bidCount\": " << book->get_bid_count() << ",\n";
    json << "    \"askCount\": " << book->get_ask_count() << ",\n";
    json << "    \"activeOrders\": " << (book->get_bid_count() + book->get_ask_count()) << "\n";
    json << "  }\n";

    json << "}\n";

    // Write to file

    fs::path dataPath;


    fs::path currentPath = fs::current_path();
    if (currentPath.filename().string().find("cmake-build") == 0) {

        dataPath = currentPath.parent_path() / "visualization" / "data";
    } else {

        dataPath = currentPath / "visualization" / "data";
    }

    // Create directory if needed
    fs::create_directories(dataPath);

    // Write the file
    fs::path jsonFile = dataPath / "orderbook.json";
    std::ofstream file(jsonFile);
    if (file.is_open()) {
        file << json.str();
        file.close();
    } else {
        std::cerr << "ERROR: Could not write to " << jsonFile << std::endl;
    }
}

int main() {
    std::cout << "Order Matching Engine - B-Tree Implementation\n";
    std::cout << "=============================================\n\n";

    // Show where we are
    std::cout << "Current directory: " << fs::current_path() << "\n\n";

    // Create the matching engine
    MatchingEngine engine;
    engine.create_order_book("AAPL", std::make_unique<BTreeOrderBook>("AAPL"));

    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> qty_dist(100, 1000);

    int orderId = 1;

    // Market parameters
    double midPrice = 150.0;
    double spreadSize = 0.10;  // 10 cents spread minimum

    // Add initial orders with proper spread
    std::cout << "Adding initial orders...\n";

    // Add buy orders (bids) - these should be BELOW mid price
    for (int i = 0; i < 10; i++) {
        double price = midPrice - spreadSize - (i * 0.05);  // Start at 149.90, decrease by 5 cents
        auto buyOrder = std::make_shared<Order>(
            orderId++, BUY, price, qty_dist(gen), "AAPL"
        );
        engine.submit_order(buyOrder);
    }

    // Add sell orders (asks) - these should be ABOVE mid price
    for (int i = 0; i < 10; i++) {
        double price = midPrice + spreadSize + (i * 0.05);  // Start at 150.10, increase by 5 cents
        auto sellOrder = std::make_shared<Order>(
            orderId++, SELL, price, qty_dist(gen), "AAPL"
        );
        engine.submit_order(sellOrder);
    }

    std::cout << "Initial orders added.\n";
    std::cout << "Best Bid: $" << engine.get_best_bid("AAPL") << "\n";
    std::cout << "Best Ask: $" << engine.get_best_ask("AAPL") << "\n";
    std::cout << "Starting continuous updates...\n";
    std::cout << "Open http://localhost:8080 in your browser\n\n";

    // Write initial state
    writeOrderBookToJson(engine, "AAPL");

    // Main loop - run for 60 seconds
    auto startTime = std::chrono::steady_clock::now();
    int updateCount = 0;

    while (true) {
        // Generate new orders that maintain the spread
        for (int i = 0; i < 3; i++) {  // Fewer orders per update
            Side side = (gen() % 2 == 0) ? BUY : SELL;
            double price;

            if (side == BUY) {
                // Buy orders: distribute around (midPrice - spread)
                double basePrice = midPrice - spreadSize;
                double variation = std::uniform_real_distribution<>(0, 1.0)(gen);
                price = basePrice - variation;
            } else {
                // Sell orders: distribute around (midPrice + spread)
                double basePrice = midPrice + spreadSize;
                double variation = std::uniform_real_distribution<>(0, 1.0)(gen);
                price = basePrice + variation;
            }

            price = std::round(price * 100) / 100;  // Round to cents
            int quantity = qty_dist(gen);

            auto order = std::make_shared<Order>(orderId++, side, price, quantity, "AAPL");
            engine.submit_order(order);
        }

        // Occasionally add an aggressive order that might match
        if (gen() % 10 == 0) {  // 10% chance
            Side side = (gen() % 2 == 0) ? BUY : SELL;
            double price;

            if (side == BUY) {
                // Aggressive buy: at or slightly above mid price
                price = midPrice + std::uniform_real_distribution<>(-0.05, 0.15)(gen);
            } else {
                // Aggressive sell: at or slightly below mid price
                price = midPrice - std::uniform_real_distribution<>(-0.05, 0.15)(gen);
            }

            price = std::round(price * 100) / 100;
            auto order = std::make_shared<Order>(orderId++, side, price, qty_dist(gen), "AAPL");
            engine.submit_order(order);
        }

        // Try to match orders
        auto trades = engine.match_orders("AAPL");
        if (!trades.empty()) {
            std::cout << "Matched " << trades.size() << " trades at ";
            for (const auto& trade : trades) {
                std::cout << "$" << trade.get_price() << " ";
            }
            std::cout << "\n";
        }

        // Update mid price with small random walk
        midPrice += std::uniform_real_distribution<>(-0.02, 0.02)(gen);
        midPrice = std::max(145.0, std::min(155.0, midPrice)); // Keep in reasonable range

        // Write current state to JSON
        writeOrderBookToJson(engine, "AAPL");

        // Show progress
        updateCount++;
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

        if (updateCount % 5 == 0) {  // Print stats every 5 updates
            std::cout << "\r[" << elapsed << "s] Orders: " << orderId - 1
                      << " | Best Bid: $" << std::fixed << std::setprecision(2)
                      << engine.get_best_bid("AAPL")
                      << " | Best Ask: $" << engine.get_best_ask("AAPL")
                      << " | Spread: $" << (engine.get_best_ask("AAPL") - engine.get_best_bid("AAPL"))
                      << "   " << std::flush;
        }

        // Wait before next update
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Stop after 60 seconds
        if (elapsed >= 60) break;
    }

    std::cout << "\n\nDemo complete!\n";
    std::cout << "Total orders processed: " << orderId - 1 << "\n";

    return 0;
}