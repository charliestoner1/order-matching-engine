#include "BTreeOrderBook.h"

namespace order_matching {

BTreeOrderBook::BTreeOrderBook(const std::string& symbol, size_t degree)
    : degree_(degree),
      min_keys_(degree - 1),
      max_keys_(2 * degree - 1) {
    symbol_ = symbol;

    // Initialize empty B-Tree roots
    buy_tree_root_ = std::make_unique<BTreeNode>();
    sell_tree_root_ = std::make_unique<BTreeNode>();
}

bool BTreeOrderBook::add_order(std::shared_ptr<Order> order) {
    if (!order || order->get_symbol() != symbol_) {
        return false;
    }

    // Record order location for fast cancellation
    order_location_[order->get_order_id()] = {order->get_side(), order->get_price()};

    // Add to appropriate tree
    if (order->get_side() == Side::BUY) {
        if (buy_tree_root_->num_keys == max_keys_) {
            // Root is full, need to split
            auto new_root = std::make_unique<BTreeNode>();
            new_root->is_leaf = false;
            new_root->children[0] = std::move(buy_tree_root_);
            split_child(new_root.get(), 0, new_root->children[0].get());
            buy_tree_root_ = std::move(new_root);
        }
        insert_non_full(buy_tree_root_.get(), order->get_price(), order, true);
        ++bid_count_;
    } else {
        if (sell_tree_root_->num_keys == max_keys_) {
            // Root is full, need to split
            auto new_root = std::make_unique<BTreeNode>();
            new_root->is_leaf = false;
            new_root->children[0] = std::move(sell_tree_root_);
            split_child(new_root.get(), 0, new_root->children[0].get());
            sell_tree_root_ = std::move(new_root);
        }
        insert_non_full(sell_tree_root_.get(), order->get_price(), order, false);
        ++ask_count_;
    }

    ++total_orders_;
    return true;
}

bool BTreeOrderBook::cancel_order(Order::OrderId order_id) {
    auto it = order_location_.find(order_id);
    if (it == order_location_.end()) {
        return false;
    }

    auto [side, price] = it->second;
    order_location_.erase(it);

    // Find and cancel the order
    BTreeNode* root = (side == Side::BUY) ? buy_tree_root_.get() : sell_tree_root_.get();
    PriceLevel* level = search(root, price);

    if (level) {
        auto& orders = level->orders;
        for (auto order_it = orders.begin(); order_it != orders.end(); ++order_it) {
            if ((*order_it)->get_order_id() == order_id) {
                (*order_it)->cancel();
                orders.erase(order_it);
                if (side == Side::BUY) --bid_count_;
                else --ask_count_;
                return true;
            }
        }
    }

    return false;
}

std::vector<Trade> BTreeOrderBook::match_orders() {
    std::vector<Trade> trades;

    while (bid_count_ > 0 && ask_count_ > 0) {
        // Get best bid and ask
        PriceLevel* best_bid = find_max(buy_tree_root_.get());
        PriceLevel* best_ask = find_min(sell_tree_root_.get());

        if (!best_bid || !best_ask || best_bid->orders.empty() || best_ask->orders.empty()) {
            break;
        }

        if (best_bid->price < best_ask->price) {
            break;  // No match possible
        }

        // Match orders
        auto& buy_order = best_bid->orders.front();
        auto& sell_order = best_ask->orders.front();

        double match_quantity = std::min(
            buy_order->get_remaining_quantity(),
            sell_order->get_remaining_quantity()
        );

        trades.emplace_back(
            generate_trade_id(),
            buy_order->get_order_id(),
            sell_order->get_order_id(),
            best_ask->price,  // Trade at ask price
            match_quantity,
            symbol_
        );

        // Update quantities
        buy_order->set_remaining_quantity(
            buy_order->get_remaining_quantity() - match_quantity);
        sell_order->set_remaining_quantity(
            sell_order->get_remaining_quantity() - match_quantity);

        // Remove filled orders
        if (buy_order->get_status() == OrderStatus::FILLED) {
            order_location_.erase(buy_order->get_order_id());
            best_bid->orders.pop_front();
            --bid_count_;

            if (best_bid->orders.empty()) {
                remove(buy_tree_root_.get(), best_bid->price);
            }
        }

        if (sell_order->get_status() == OrderStatus::FILLED) {
            order_location_.erase(sell_order->get_order_id());
            best_ask->orders.pop_front();
            --ask_count_;

            if (best_ask->orders.empty()) {
                remove(sell_tree_root_.get(), best_ask->price);
            }
        }
    }

    total_trades_ += trades.size();
    return trades;
}

// B-Tree operation stubs
void BTreeOrderBook::split_child(BTreeNode* parent, size_t index, BTreeNode* child) {
    // TODO: Implement B-Tree split operation
}

void BTreeOrderBook::insert_non_full(BTreeNode* node, double price, std::shared_ptr<Order> order, bool is_buy_side) {
    // TODO: Implement B-Tree insertion in non-full node
}

BTreeOrderBook::PriceLevel* BTreeOrderBook::search(BTreeNode* node, double price) const {
    // TODO: Implement B-Tree search
    return nullptr;
}

void BTreeOrderBook::remove(BTreeNode* node, double price) {
    // TODO: Implement B-Tree deletion
}

BTreeOrderBook::PriceLevel* BTreeOrderBook::find_min(BTreeNode* node) const {
    // TODO: Find minimum price level in B-Tree
    return nullptr;
}

BTreeOrderBook::PriceLevel* BTreeOrderBook::find_max(BTreeNode* node) const {
    // TODO: Find maximum price level in B-Tree
    return nullptr;
}

// Query operations
size_t BTreeOrderBook::get_bid_count() const noexcept {
    return bid_count_;
}

size_t BTreeOrderBook::get_ask_count() const noexcept {
    return ask_count_;
}

std::optional<double> BTreeOrderBook::get_best_bid() const noexcept {
    PriceLevel* best = find_max(buy_tree_root_.get());
    return best ? std::optional<double>(best->price) : std::nullopt;
}

std::optional<double> BTreeOrderBook::get_best_ask() const noexcept {
    PriceLevel* best = find_min(sell_tree_root_.get());
    return best ? std::optional<double>(best->price) : std::nullopt;
}

std::optional<double> BTreeOrderBook::get_spread() const noexcept {
    auto bid = get_best_bid();
    auto ask = get_best_ask();

    if (bid.has_value() && ask.has_value()) {
        return ask.value() - bid.value();
    }
    return std::nullopt;
}

std::vector<OrderBook::Level> BTreeOrderBook::get_bid_levels(size_t max_levels) const {
    std::vector<Level> levels;
    size_t count = 0;
    collect_levels(buy_tree_root_.get(), levels, count, max_levels, true);
    return levels;
}

std::vector<OrderBook::Level> BTreeOrderBook::get_ask_levels(size_t max_levels) const {
    std::vector<Level> levels;
    size_t count = 0;
    collect_levels(sell_tree_root_.get(), levels, count, max_levels, false);
    return levels;
}

void BTreeOrderBook::collect_levels(BTreeNode* node, std::vector<Level>& levels,
                                   size_t& count, size_t max_levels, bool reverse) const {
    // TODO: Implement level collection for order book snapshot
}

size_t BTreeOrderBook::get_total_orders() const noexcept {
    return total_orders_;
}

size_t BTreeOrderBook::get_total_trades() const noexcept {
    return total_trades_;
}

} // namespace order_matching