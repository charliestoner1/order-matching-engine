#pragma once

#include "../core/OrderBook.h"
#include <memory>
#include <unordered_map>
#include <array>
#include <deque>
#include <atomic>

namespace order_matching {

// B-Tree implementation optimized for cache performance
class BTreeOrderBook : public OrderBook {
public:
    explicit BTreeOrderBook(const std::string& symbol, size_t degree = 64);
    ~BTreeOrderBook() override = default;

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
    static constexpr size_t MAX_DEGREE = 256;  // Maximum B-Tree degree

    // Price level with orders
    struct PriceLevel {
        double price{0.0};
        std::deque<std::shared_ptr<Order>> orders;

        PriceLevel() = default;  // Default constructor
        explicit PriceLevel(double p) : price(p) {}
    };

    // B-Tree node optimized for cache line size
    struct alignas(64) BTreeNode {
        size_t num_keys{0};
        bool is_leaf{true};

        // Arrays sized for cache efficiency
        std::array<PriceLevel, MAX_DEGREE - 1> keys;
        std::array<std::unique_ptr<BTreeNode>, MAX_DEGREE> children;

        BTreeNode() = default;
    };

    using BTreeNodePtr = std::unique_ptr<BTreeNode>;

    const size_t degree_;  // B-Tree degree (t)
    const size_t min_keys_;  // Minimum keys (t-1)
    const size_t max_keys_;  // Maximum keys (2t-1)

    // B-Tree roots
    BTreeNodePtr buy_tree_root_;   // Buy side (descending order)
    BTreeNodePtr sell_tree_root_;  // Sell side (ascending order)

    // Order lookup for cancellation
    std::unordered_map<Order::OrderId, std::pair<Side, double>> order_location_;

    // Metrics
    std::atomic<size_t> total_orders_{0};
    std::atomic<size_t> total_trades_{0};
    size_t bid_count_{0};
    size_t ask_count_{0};

    // B-Tree operations
    void split_child(BTreeNode* parent, size_t index, BTreeNode* child);
    void insert_non_full(BTreeNode* node, double price, std::shared_ptr<Order> order, bool is_buy_side);
    void merge(BTreeNode* node, size_t index);
    void borrow_from_prev(BTreeNode* node, size_t index);
    void borrow_from_next(BTreeNode* node, size_t index);
    void fill(BTreeNode* node, size_t index);

    PriceLevel* search(BTreeNode* node, double price) const;
    void remove(BTreeNode* node, double price);
    PriceLevel* find_min(BTreeNode* node) const;
    PriceLevel* find_max(BTreeNode* node) const;

    // Helper functions
    void collect_levels(BTreeNode* node, std::vector<Level>& levels, size_t& count, size_t max_levels, bool reverse) const;
    int find_key(BTreeNode* node, double price, bool is_buy_side) const;
};

} // namespace order_matching