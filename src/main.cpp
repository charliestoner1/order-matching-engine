#include <iostream>
#include <memory>
#include "core/Order.h"

using namespace order_matching;

int main() {
    std::cout << "Order Matching Engine v1.0.0" << std::endl;
    std::cout << "==========================" << std::endl;

    // Test order creation
    Order test_order(1001, Side::BUY, 50000.0, 1.5, "BTCUSD");

    std::cout << "\nTest Order Created:" << std::endl;
    std::cout << "  Order ID: " << test_order.get_order_id() << std::endl;
    std::cout << "  Side: " << to_string(test_order.get_side()) << std::endl;
    std::cout << "  Price: $" << test_order.get_price() << std::endl;
    std::cout << "  Quantity: " << test_order.get_quantity() << std::endl;
    std::cout << "  Symbol: " << test_order.get_symbol() << std::endl;
    std::cout << "  Status: " << to_string(test_order.get_status()) << std::endl;

    return 0;
}