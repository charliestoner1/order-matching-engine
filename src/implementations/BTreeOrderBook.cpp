#include "BTreeOrderBook.h"

namespace order_matching {

BTreeOrderBook::BTreeOrderBook(const std::string& symbol, size_t degree)
    : degree_(degree),
      min_keys_(degree - 1),
      max_keys_(2 * degree - 1),
      bid_count_(0),
      ask_count_(0),
      total_orders_(0),
      total_trades_(0) {
    symbol_ = symbol;

    // Initialize empty B-Tree roots
    buy_tree_root_ = new BTreeNode();
    sell_tree_root_ = new BTreeNode();
}


bool BTreeOrderBook::add_order(std::shared_ptr<Order> order) {
    if (!order || order->get_symbol() != symbol_) {
        return false;
    }

    // TODO: Implement B-Tree insertion
    // For now-increment counters
    if (order->get_side() == BUY) {
        ++bid_count_;
    } else {
        ++ask_count_;
    }
    ++total_orders_;

    return true;
}

bool BTreeOrderBook::cancel_order(Order::OrderId order_id) {
    // TODO: Implement order cancellation
    return false;
}

std::vector<Trade> BTreeOrderBook::match_orders() {
    std::vector<Trade> trades;
    // TODO: Implement order matching
    return trades;
}

double BTreeOrderBook::get_best_bid() const {
    return find_best_price(buy_tree_root_, true);
}

double BTreeOrderBook::get_best_ask() const {
    return find_best_price(sell_tree_root_, false);
}

size_t BTreeOrderBook::get_bid_count() const {
    return bid_count_;
}

size_t BTreeOrderBook::get_ask_count() const {
    return ask_count_;
}

size_t BTreeOrderBook::get_total_orders() const {
    return total_orders_;
}

std::vector<OrderBook::Level> BTreeOrderBook::get_bid_levels(size_t max_levels) const {
    std::vector<Level> levels;
    // TODO: Implement level collection
    return levels;
}

std::vector<OrderBook::Level> BTreeOrderBook::get_ask_levels(size_t max_levels) const {
    std::vector<Level> levels;
    // TODO: Implement level collection
    return levels;
}

// B-Tree helper methods
void BTreeOrderBook::insert(BTreeNode*& root, double price, std::shared_ptr<Order> order, bool is_buy_side) {
    if (root == nullptr) {
        root = new BTreeNode();
        root->keys.emplace_back(price);
        root->keys.back().orders.push_back(order);
    }
    // need to split
    if (root->keys.size() == max_keys_) {
        BTreeNode* newRoot = new BTreeNode();
        newRoot->is_leaf = false;
        newRoot->children.push_back(root);
        split_child(newRoot, 0);
        root = newRoot;
    }
    // if the root is not full
    BTreeNode* current = root;
    while (!current->is_leaf) {
        int i = current->keys.size() - 1;
        while (price < current->keys[i].price && i >= 0) {
            i--;
        }
        // gets correct index for insertion
        i++;
        // splits lower levels if needed
        if (current->children[i]->keys.size() == max_keys_) {
            split_child(current, i);
            // one child moves up, so determine which is the right one
            if (price > current->keys[i].price) {
                i++;
            }
        }
        current = current->children[i];
    }
    // now at leaf node
    int i = current->keys.size() - 1;
    while (price < current->keys[i].price && i >= 0) {
        i--;
    }
    if (current->keys[i].price == price) {
        current->keys[i].orders.push_back(order);
    }
    // otherwise make a new price level
    else {
        PriceLevel newPrice(price);
        newPrice.orders.push_back(order);
        auto it = current->keys.begin() + i + 1;
        current->keys.insert(it, newPrice);
    }
}

BTreeOrderBook::BTreeNode* BTreeOrderBook::search(BTreeNode* root, double price) const {
    BTreeNode* current = root;
    while (current != nullptr) {
        size_t i = 0;
        while (price > current->keys[i].price && i < current->keys.size()) {
            i++;
        }
        if (price == current->keys[i].price && i < current->keys.size()) {
            return current;
        }
        // if leaf searched and price not found, does not exist
        if (current->is_leaf) {
            return nullptr;
        }
        current = current->children[i];
    }
    // if reached here it does not exist
    return nullptr;
}

void BTreeOrderBook::split_child(BTreeNode* parent, int index) {
    BTreeNode* nodeToSplit = parent->children[index];
    // always odd when splitting, move median up and make the two halves separate
    int mid = nodeToSplit->keys.size()/2;
    BTreeNode* newNode = new BTreeNode();
    newNode->is_leaf = nodeToSplit->is_leaf;

    PriceLevel midKey = nodeToSplit->keys[mid];
    newNode->keys.assign(nodeToSplit->keys.begin() + mid + 1, nodeToSplit->keys.end());
    // resizes full node so that it only contains the first half
    nodeToSplit->keys.resize(mid);

    // split the children if it is an internal node
    if (!nodeToSplit->is_leaf) {
        newNode->children.assign(nodeToSplit->children.begin() + mid + 1, nodeToSplit->children.end());
        nodeToSplit->children.resize(mid);
    }
    // move the middle one up to the parent node
    parent->keys.insert(parent->keys.begin() + index, midKey);
    parent->children.insert(parent->children.begin() + index + 1, newNode);
}

BTreeOrderBook::PriceLevel* BTreeOrderBook::find_price_level(BTreeNode* root, double price) const {
    BTreeNode* current = root;
    while (current != nullptr) {
        size_t i = 0;
        while (price > current->keys[i].price && i < current->keys.size()) {
            i++;
        }
        if (price == current->keys[i].price && i < current->keys.size()) {
            return &current->keys[i];
        }
        // if leaf searched and price not found, does not exist
        if (current->is_leaf) {
            return nullptr;
        }
        current = current->children[i];
    }
    // if reached here it does not exist
    return nullptr;
}


double BTreeOrderBook::find_best_price(BTreeNode* root, bool find_max) const {
    if (root == nullptr) {
        return 0.0;
    }
    BTreeNode* current = root;
    while (!current->is_leaf) {
        if (find_max) {
            current = current->children.back();
        }
        else {
            current = current->children.front();
        }
    }
    if (current->keys.empty()) {
        return 0.0;
    }
    if (find_max) {
        return current->keys.back().price;
    }
    else {
        return current->keys.front().price;
    }
}

void BTreeOrderBook::collect_levels(BTreeNode* node, std::vector<Level>& levels, size_t& count, size_t max_levels) const {
    // TODO: Implement level collection
}

} // namespace order_matching