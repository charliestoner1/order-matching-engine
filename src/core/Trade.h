#pragma once

#include <string>

namespace order_matching {

    class Trade {
    public:
        typedef unsigned long TradeId;
        typedef unsigned long OrderId;

    private:
        TradeId trade_id;
        OrderId buy_order_id;
        OrderId sell_order_id;
        double price;
        double quantity;
        std::string symbol;
        long timestamp;

    public:
        Trade(TradeId id, OrderId buy_id, OrderId sell_id,
              double p, double qty, const std::string& sym)
            : trade_id(id), buy_order_id(buy_id), sell_order_id(sell_id),
              price(p), quantity(qty), symbol(sym) {
            timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        }

        // getters
        TradeId get_trade_id() const { return trade_id; }
        OrderId get_buy_order_id() const { return buy_order_id; }
        OrderId get_sell_order_id() const { return sell_order_id; }
        double get_price() const { return price; }
        double get_quantity() const { return quantity; }
        const std::string& get_symbol() const { return symbol; }
        long get_timestamp() const { return timestamp; }
    };

} // namespace order_matching