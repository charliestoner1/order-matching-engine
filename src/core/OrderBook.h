#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <atomic>
#include "Order.h"
#include "Trade.h"

namespace order_matching {

    class OrderBook {
    public:
        virtual ~OrderBook() = default;

        // Core operations
        virtual bool add_order(std::shared_ptr<Order> order) = 0;
        virtual bool cancel_order(Order::OrderId order_id) = 0;
        virtual std::vector<Trade> match_orders() = 0;

        // Query operations
        [[nodiscard]] virtual size_t get_bid_count() const noexcept = 0;
        [[nodiscard]] virtual size_t get_ask_count() const noexcept = 0;
        [[nodiscard]] virtual std::optional<double> get_best_bid() const noexcept = 0;
        [[nodiscard]] virtual std::optional<double> get_best_ask() const noexcept = 0;
        [[nodiscard]] virtual std::optional<double> get_spread() const noexcept = 0;

        // Order book snapshot
        struct Level {
            double price;
            double quantity;
            size_t order_count;
        };

        [[nodiscard]] virtual std::vector<Level> get_bid_levels(size_t max_levels = 10) const = 0;
        [[nodiscard]] virtual std::vector<Level> get_ask_levels(size_t max_levels = 10) const = 0;

        // Performance metrics
        [[nodiscard]] virtual size_t get_total_orders() const noexcept = 0;
        [[nodiscard]] virtual size_t get_total_trades() const noexcept = 0;

    protected:
        // Symbol for this order book
        std::string symbol_;

        // Trade ID generator
        static std::atomic<Trade::TradeId> next_trade_id_;

        [[nodiscard]] Trade::TradeId generate_trade_id() noexcept {
            return next_trade_id_.fetch_add(1, std::memory_order_relaxed);
        }
    };

} // namespace order_matching