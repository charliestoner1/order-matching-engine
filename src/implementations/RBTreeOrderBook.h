#pragma once

#include "../core/OrderBook.h"
#include <memory>
#include <unordered_map>
#include <deque>
#include <atomic>

namespace order_matching {

// Red-Black Tree Node
template<typename T>
struct RBNode {
    enum Color { RED, BLACK };

    T data;
    Color color;
    std::shared_ptr<RBNode> left;
    std::shared_ptr<RBNode> right;
    std::shared_ptr<RBNode> parent;

    explicit RBNode(T value)
        : data(std::move(value)), color(RED), left(nullptr), right(nullptr), parent(nullptr) {}
};

// Red-Black Tree implementation for Order Book
class RBTreeOrderBook : public OrderBook {
public:
    explicit RBTreeOrderBook(const std::string& symbol);
    ~RBTreeOrderBook() override = default;

    // Core operations
    bool add_order(std::shared_ptr<Order> order) override;
    bool cancel_order(Order::OrderId order_id) override;
    std::vector<Trade> match_orders() override;

    // Query operations
    [[nodiscard]] size_t get_bid_count() const noexcept override;
    [[nodiscard]] size_t get_ask_count() const noexcept override;
    [[nodiscard]] std::optional<double> get_best_bid() const noexcept override;
    [[nodiscard]] std::optional<double> get_best_ask() const noexcept override;
    [[nodiscard]] std::optional<double> get_spread() const noexcept override;

    // Order book snapshot
    [[nodiscard]] std::vector<Level> get_bid_levels(size_t max_levels = 10) const override;
    [[nodiscard]] std::vector<Level> get_ask_levels(size_t max_levels = 10) const override;

    // Performance metrics
    [[nodiscard]] size_t get_total_orders() const noexcept override;
    [[nodiscard]] size_t get_total_trades() const noexcept override;

private:
    // Price level contains orders at that price (FIFO queue)
    struct PriceLevel {
        double price;
        std::deque<std::shared_ptr<Order>> orders;

        explicit PriceLevel(double p) : price(p) {}
    };

    using RBNodePtr = std::shared_ptr<RBNode<PriceLevel>>;

    // Red-Black Tree roots
    RBNodePtr buy_tree_root_;   // Max heap property (highest price at leftmost)
    RBNodePtr sell_tree_root_;  // Min heap property (lowest price at leftmost)

    // Order lookup map for O(1) cancellation
    std::unordered_map<Order::OrderId, std::pair<Side, double>> order_location_;

    // Metrics
    std::atomic<size_t> total_orders_{0};
    std::atomic<size_t> total_trades_{0};
    size_t bid_count_{0};
    size_t ask_count_{0};

    // Red-Black Tree operations
    void rotate_left(RBNodePtr& root, RBNodePtr node);
    void rotate_right(RBNodePtr& root, RBNodePtr node);
    void fix_insert_violation(RBNodePtr& root, RBNodePtr node);
    void fix_delete_violation(RBNodePtr& root, RBNodePtr node);

    RBNodePtr insert_node(RBNodePtr& root, double price, bool is_buy_side);
    RBNodePtr find_node(RBNodePtr root, double price) const;
    RBNodePtr find_min(RBNodePtr node) const;
    RBNodePtr find_max(RBNodePtr node) const;
    void delete_node(RBNodePtr& root, RBNodePtr node);

    // Helper functions
    void collect_levels(RBNodePtr node, std::vector<Level>& levels, size_t& count, size_t max_levels) const;
};

} // namespace order_matching