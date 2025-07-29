#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace order_matching {

    class Trade {
    public:
        using TradeId = uint64_t;
        using OrderId = uint64_t;
        using Timestamp = std::chrono::nanoseconds;

        Trade(TradeId id, OrderId buy_order_id, OrderId sell_order_id,
              double price, double quantity, const std::string& symbol)
            : trade_id_(id)
            , buy_order_id_(buy_order_id)
            , sell_order_id_(sell_order_id)
            , price_(price)
            , quantity_(quantity)
            , symbol_(symbol)
            , timestamp_(std::chrono::steady_clock::now().time_since_epoch()) {}

        // Getters
        [[nodiscard]] TradeId get_trade_id() const noexcept { return trade_id_; }
        [[nodiscard]] OrderId get_buy_order_id() const noexcept { return buy_order_id_; }
        [[nodiscard]] OrderId get_sell_order_id() const noexcept { return sell_order_id_; }
        [[nodiscard]] double get_price() const noexcept { return price_; }
        [[nodiscard]] double get_quantity() const noexcept { return quantity_; }
        [[nodiscard]] const std::string& get_symbol() const noexcept { return symbol_; }
        [[nodiscard]] Timestamp get_timestamp() const noexcept { return timestamp_; }

    private:
        TradeId trade_id_;
        OrderId buy_order_id_;
        OrderId sell_order_id_;
        double price_;
        double quantity_;
        std::string symbol_;
        Timestamp timestamp_;
    };

} // namespace order_matching