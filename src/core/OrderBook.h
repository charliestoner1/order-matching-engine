#pragma once

#include <memory>
#include <vector>
#include "Order.h"
#include "Trade.h"

namespace order_matching {

    class OrderBook {
    public:
        virtual ~OrderBook() {}

        // Core operations
        virtual bool add_order(std::shared_ptr<Order> order) = 0;
        virtual bool cancel_order(Order::OrderId order_id) = 0;
        virtual std::vector<Trade> match_orders() = 0;

        // queries - return 0 if no orders
        virtual double get_best_bid() const = 0;
        virtual double get_best_ask() const = 0;
        virtual size_t get_bid_count() const = 0;
        virtual size_t get_ask_count() const = 0;
        virtual size_t get_total_orders() const = 0;

        // Order book levels for display
        struct Level {
            double price;
            double quantity;
            size_t order_count;

            Level(double p, double qty, size_t orders)
                : price(p), quantity(qty), order_count(orders) {}
        };

        virtual std::vector<Level> get_bid_levels(size_t max_levels = 10) const = 0;
        virtual std::vector<Level> get_ask_levels(size_t max_levels = 10) const = 0;

    protected:
        std::string symbol_;
        static unsigned long next_trade_id;

        Trade::TradeId generate_trade_id() {
            return next_trade_id++;
        }
    };

} // namespace order_matching