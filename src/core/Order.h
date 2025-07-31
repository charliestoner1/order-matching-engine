#pragma once

#include <chrono>
#include <string>

namespace order_matching {

    enum Side { BUY, SELL };

    enum OrderStatus { NEW, PARTIALLY_FILLED, FILLED, CANCELLED };

    class Order {
    public:
        typedef unsigned long OrderId;

    private:
        OrderId order_id;
        Side side;
        double price;
        double quantity;
        double remaining_quantity;
        std::string symbol;
        OrderStatus status;
        long timestamp;  // Simple timestamp

    public:
        Order(OrderId id, Side s, double p, double qty, const std::string& sym)
            : order_id(id), side(s), price(p), quantity(qty),
              remaining_quantity(qty), symbol(sym), status(NEW) {
            timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        }

        // getters
        OrderId get_order_id() const { return order_id; }
        Side get_side() const { return side; }
        double get_price() const { return price; }
        double get_quantity() const { return quantity; }
        double get_remaining_quantity() const { return remaining_quantity; }
        const std::string& get_symbol() const { return symbol; }
        OrderStatus get_status() const { return status; }
        long get_timestamp() const { return timestamp; }

        // setters
        void set_remaining_quantity(double qty) {
            remaining_quantity = qty;
            if (remaining_quantity <= 0) {
                status = FILLED;
            } else if (remaining_quantity < quantity) {
                status = PARTIALLY_FILLED;
            }
        }

        void cancel() {
            status = CANCELLED;
        }

        bool is_filled() const {
            return remaining_quantity <= 0;
        }

        // For price-time priority
        bool has_priority_over(const Order& other) const {
            if (price != other.price) {
                return (side == BUY) ? (price > other.price) : (price < other.price);
            }
            return timestamp < other.timestamp;
        }
    };

} // namespace order_matching