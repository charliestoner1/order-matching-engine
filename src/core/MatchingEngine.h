#pragma once

#include <memory>
#include <map>
#include <string>
#include "OrderBook.h"
#include "Order.h"
#include "Trade.h"

namespace order_matching {

//
// matching engine that manages multiple order books (one per symbol: ie. AAPL, TSLA)
class MatchingEngine {
private:
    // Order books by symbol
    std::map<std::string, std::unique_ptr<OrderBook>> order_books_;

public:
    MatchingEngine() {}
    ~MatchingEngine() {}

    // Set OrderBook
    void create_order_book(const std::string& symbol, std::unique_ptr<OrderBook> book) {
        order_books_[symbol] = std::move(book);
    }

    // Get OrderBook (needed for export)
    OrderBook* get_order_book(const std::string& symbol) {
        auto it = order_books_.find(symbol);
        if (it != order_books_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    // Submit an order to the appropriate book
    bool submit_order(std::shared_ptr<Order> order) {
        auto it = order_books_.find(order->get_symbol());
        if (it != order_books_.end()) {
            return it->second->add_order(order);
        }
        return false; // No book for this symbol
    }

    // Cancel an order
    bool cancel_order(const std::string& symbol, Order::OrderId order_id) {
        auto it = order_books_.find(symbol);
        if (it != order_books_.end()) {
            return it->second->cancel_order(order_id);
        }
        return false;
    }

    // Run matching for a specific symbol
    std::vector<Trade> match_orders(const std::string& symbol) {
        auto it = order_books_.find(symbol);
        if (it != order_books_.end()) {
            return it->second->match_orders();
        }
        return {};
    }

    // Get market data
    double get_best_bid(const std::string& symbol) const {
        auto it = order_books_.find(symbol);
        if (it != order_books_.end()) {
            return it->second->get_best_bid();
        }
        return 0.0;
    }

    double get_best_ask(const std::string& symbol) const {
        auto it = order_books_.find(symbol);
        if (it != order_books_.end()) {
            return it->second->get_best_ask();
        }
        return 0.0;
    }
};

} // namespace order_matching