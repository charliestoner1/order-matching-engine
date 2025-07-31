#include <iostream>
#include "core/MatchingEngine.h"
#include "implementations/BTreeOrderBook.h"
using namespace order_matching;

int main() {
    std::cout << "Order Matching Engine v1.0.0" << std::endl;
    std::cout << "===========================" << std::endl;

    MatchingEngine engine;
    engine.create_order_book("AAPL", std::make_unique<BTreeOrderBook>("AAPL"));

    auto order = std::make_shared<Order>(1, BUY, 100.0, 10, "AAPL");
    if (engine.submit_order(order)) {
        std::cout << "Order submitted successfully!" << std::endl;
        std::cout << "Best bid: $" << engine.get_best_bid("AAPL") << std::endl;
    }

    return 0;
}