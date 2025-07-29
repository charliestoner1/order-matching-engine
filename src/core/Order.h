#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace order_matching {

enum class Side : uint8_t {
    BUY = 0,
    SELL = 1
};

enum class OrderStatus : uint8_t {
    NEW = 0,
    PARTIALLY_FILLED = 1,
    FILLED = 2,
    CANCELLED = 3
};

class Order {
public:
    using OrderId = uint64_t;
    using Timestamp = std::chrono::nanoseconds;

    Order() = default;

    Order(OrderId id, Side side, double price, double quantity, const std::string& symbol)
        : order_id_(id)
        , timestamp_(std::chrono::steady_clock::now().time_since_epoch())
        , side_(side)
        , price_(price)
        , quantity_(quantity)
        , remaining_quantity_(quantity)
        , symbol_(symbol)
        , status_(OrderStatus::NEW) {}

    // Getters
    [[nodiscard]] OrderId get_order_id() const noexcept { return order_id_; }
    [[nodiscard]] Timestamp get_timestamp() const noexcept { return timestamp_; }
    [[nodiscard]] Side get_side() const noexcept { return side_; }
    [[nodiscard]] double get_price() const noexcept { return price_; }
    [[nodiscard]] double get_quantity() const noexcept { return quantity_; }
    [[nodiscard]] double get_remaining_quantity() const noexcept { return remaining_quantity_; }
    [[nodiscard]] const std::string& get_symbol() const noexcept { return symbol_; }
    [[nodiscard]] OrderStatus get_status() const noexcept { return status_; }

    // Modifiers
    void set_remaining_quantity(double qty) noexcept {
        remaining_quantity_ = qty;
        if (remaining_quantity_ <= 0.0) {
            status_ = OrderStatus::FILLED;
        } else if (remaining_quantity_ < quantity_) {
            status_ = OrderStatus::PARTIALLY_FILLED;
        }
    }

    void cancel() noexcept { status_ = OrderStatus::CANCELLED; }

    // Comparison operators for price-time priority
    [[nodiscard]] bool has_priority_over(const Order& other) const noexcept {
        if (price_ != other.price_) {
            return (side_ == Side::BUY) ? (price_ > other.price_) : (price_ < other.price_);
        }
        return timestamp_ < other.timestamp_;
    }

private:
    OrderId order_id_{};
    Timestamp timestamp_{};
    Side side_{};
    double price_{};
    double quantity_{};
    double remaining_quantity_{};
    std::string symbol_{};
    OrderStatus status_{};
};

// Helper functions
[[nodiscard]] inline const char* to_string(Side side) noexcept {
    return side == Side::BUY ? "BUY" : "SELL";
}

[[nodiscard]] inline const char* to_string(OrderStatus status) noexcept {
    switch (status) {
        case OrderStatus::NEW: return "NEW";
        case OrderStatus::PARTIALLY_FILLED: return "PARTIALLY_FILLED";
        case OrderStatus::FILLED: return "FILLED";
        case OrderStatus::CANCELLED: return "CANCELLED";
        default: return "UNKNOWN";
    }
}

} // namespace order_matching