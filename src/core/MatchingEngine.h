#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <chrono>
#include <functional>
#include "OrderBook.h"
#include "Order.h"
#include "Trade.h"

namespace order_matching {

class MatchingEngine {
public:
    using OrderBookFactory = std::function<std::unique_ptr<OrderBook>()>;
    using TradeCallback = std::function<void(const Trade&)>;
    
    explicit MatchingEngine(OrderBookFactory factory)
        : order_book_factory_(std::move(factory)) {}
    
    // Order management
    bool submit_order(std::shared_ptr<Order> order);
    bool cancel_order(const std::string& symbol, Order::OrderId order_id);
    
    // Market data
    [[nodiscard]] std::optional<double> get_best_bid(const std::string& symbol) const;
    [[nodiscard]] std::optional<double> get_best_ask(const std::string& symbol) const;
    [[nodiscard]] std::optional<double> get_spread(const std::string& symbol) const;
    
    // Order book snapshot
    [[nodiscard]] std::vector<OrderBook::Level> get_bid_levels(
        const std::string& symbol, size_t max_levels = 10) const;
    [[nodiscard]] std::vector<OrderBook::Level> get_ask_levels(
        const std::string& symbol, size_t max_levels = 10) const;
    
    // Trade notification
    void set_trade_callback(TradeCallback callback) {
        trade_callback_ = std::move(callback);
    }
    
    // Performance metrics
    struct PerformanceMetrics {
        size_t total_orders;
        size_t total_trades;
        std::chrono::nanoseconds avg_matching_time;
        std::chrono::nanoseconds min_matching_time;
        std::chrono::nanoseconds max_matching_time;
    };
    
    [[nodiscard]] PerformanceMetrics get_performance_metrics() const;
    
private:
    // Order books by symbol
    mutable std::unordered_map<std::string, std::unique_ptr<OrderBook>> order_books_;
    
    // Factory for creating order books
    OrderBookFactory order_book_factory_;
    
    // Trade notification callback
    TradeCallback trade_callback_;
    
    // Performance tracking
    mutable std::atomic<size_t> total_orders_{0};
    mutable std::atomic<size_t> total_trades_{0};
    mutable std::atomic<std::chrono::nanoseconds::rep> total_matching_time_{0};
    mutable std::atomic<std::chrono::nanoseconds::rep> min_matching_time_{
        std::numeric_limits<std::chrono::nanoseconds::rep>::max()};
    mutable std::atomic<std::chrono::nanoseconds::rep> max_matching_time_{0};
    
    // Get or create order book for symbol
    OrderBook* get_or_create_order_book(const std::string& symbol) const;
    
    // Process trades from matching
    void process_trades(const std::vector<Trade>& trades);
};

} // namespace order_matching