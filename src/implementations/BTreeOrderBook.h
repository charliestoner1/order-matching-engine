#pragma once

#include "../core/OrderBook.h"
#include <map>
#include <deque>
#include <memory>

namespace order_matching {

class BTreeOrderBook : public OrderBook {
public:
    BTreeOrderBook(const std::string& symbol, size_t degree = 32);
    ~BTreeOrderBook() {
        if (buy_tree_root_) delete buy_tree_root_;
        if (sell_tree_root_) delete sell_tree_root_;
    }

    // OrderBook interface
    bool add_order(std::shared_ptr<Order> order) override;
    bool cancel_order(Order::OrderId order_id) override;
    std::vector<Trade> match_orders() override;

    double get_best_bid() const override;
    double get_best_ask() const override;
    size_t get_bid_count() const override;
    size_t get_ask_count() const override;
    size_t get_total_orders() const override;

    std::vector<Level> get_bid_levels(size_t max_levels = 10) const override;
    std::vector<Level> get_ask_levels(size_t max_levels = 10) const override;

private:
    // B-Tree node
    struct PriceLevel {
        double price;
        std::deque<std::shared_ptr<Order>> orders;

        PriceLevel() : price(0.0) {}
        PriceLevel(double p) : price(p) {}
    };

    struct BTreeNode {
        std::vector<PriceLevel> keys;        // Price levels in this node
        std::vector<BTreeNode*> children;    // Child pointers
        bool is_leaf;

        // links for leaf chaining
        BTreeNode* next = nullptr;
        BTreeNode* prev = nullptr;

        BTreeNode() : is_leaf(true) {
            // Pre-allocate capacity for better performance
            keys.reserve(128);      // max_keys for degree 64
            children.reserve(129);  // max_keys + 1
        }

        ~BTreeNode() {
            for (auto child : children) {
                delete child;
            }
        }
    };

    // Member variables
    const size_t degree_;           // B-Tree degree
    const size_t min_keys_;         // Minimum keys (degree-1)
    const size_t max_keys_;         // Maximum keys (2*degree-1)

    BTreeNode* buy_tree_root_;      // Buy orders tree
    BTreeNode* sell_tree_root_;     // Sell orders tree

    // For fast order lookup/cancellation
    std::map<Order::OrderId, std::pair<Side, double>> order_location_;

    // Metrics
    size_t bid_count_;
    size_t ask_count_;
    size_t total_orders_;                // Current active orders
    size_t total_orders_processed_;      // Cumulative total
    size_t total_trades_;

    // B-Tree operations
    void insert(BTreeNode*& root, double price, std::shared_ptr<Order> order);
    int binary_search_price(const std::vector<PriceLevel>& keys, double price) const;
    BTreeNode* search(BTreeNode* root, double price) const;
    void split_child(BTreeNode* parent, int index);
    PriceLevel* find_price_level(BTreeNode* root, double price) const;

    // Helper functions
    double find_best_price(BTreeNode* root, bool find_max) const;
    void collect_levels(BTreeNode* node, std::vector<Level>& levels, size_t max_levels, bool reverse) const;
    // bool is_underflow(BTreeNode* node, bool is_buy_tree) const;
};

} // namespace order_matching