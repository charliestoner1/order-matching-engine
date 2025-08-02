#include "BTreeOrderBook.h"
using namespace std;
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
    auto itr = order_location_.find(order_id);
    // return false if order is not found
    if (itr == order_location_.end()) {
        return false;
    }
    Side side = itr->second.first;
    double price = itr->second.second;

    PriceLevel* priceLvl;
    if (side == Side::BUY) {
        priceLvl = find_price_level(buy_tree_root_, price);
    }
    else {
        priceLvl = find_price_level(sell_tree_root_, price);
    }
    if (priceLvl == nullptr) {
        return false;
    }
    deque<shared_ptr<Order>>& orders = priceLvl->orders;
    for (auto it = orders.begin(); it != orders.end(); ++it) {
        if ((*it)->get_order_id() == order_id) {
            orders.erase(it);
            order_location_.erase(order_id);

            // potentially add code to remove order from the actual tree
            return true;
        }
    }
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
    size_t count = 0;
    collect_levels(sell_tree_root_, levels, count, max_levels, true);
    return levels;
}

std::vector<OrderBook::Level> BTreeOrderBook::get_ask_levels(size_t max_levels) const {
    std::vector<Level> levels;
    size_t count = 0;
    collect_levels(buy_tree_root_, levels, count, max_levels, false);
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
        int i = int(current->keys.size()) - 1;
        while (i >= 0 && price < current->keys[i].price) {
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
    int i = int(current->keys.size()) - 1;
    while (i >= 0 && price < current->keys[i].price) {
        i--;
    }
    if (i >= 0 && current->keys[i].price == price) {
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
        while (i < current->keys.size() && price > current->keys[i].price) {
            i++;
        }
        if (current->is_leaf) {
            // only returns the node if its in a leaf node
            if (i < current->keys.size() && price == current->keys[i].price) {
                return current;
            }
            else {
                // if leaf searched and price not found, does not exist
                return nullptr;
            }
        }
        current = current->children[i];
    }
    // if root is nullptr
    return nullptr;
}

void BTreeOrderBook::split_child(BTreeNode* parent, int index) {
    BTreeNode* nodeToSplit = parent->children[index];
    // always odd when splitting, move median up and make the two halves separate
    int mid = nodeToSplit->keys.size()/2;
    BTreeNode* newNode = new BTreeNode();
    newNode->is_leaf = nodeToSplit->is_leaf;

    // if leaf node, make new price for guiding and move it up, maintaining the old price with orders in the leaves
    if (nodeToSplit->is_leaf) {
        newNode->keys.assign(nodeToSplit->keys.begin() + mid, nodeToSplit->keys.end());
        // resizes full node so that it only contains the first half
        nodeToSplit->keys.resize(mid);
        // ensures that the internal nodes are simply keys for searching, all the orders are stored in the leaves
        double newPrice = newNode->keys.front().price;
        PriceLevel newKey(newPrice);
        parent->keys.insert(parent->keys.begin() + index, newKey);
        parent->children.insert(parent->children.begin() + index + 1, newNode);
    }
    // if internal node, move the price level up and no need to keep it  in the split nodes (no information)
    else {
        PriceLevel newKey = nodeToSplit->keys[mid];
        newNode->keys.assign(nodeToSplit->keys.begin() + mid + 1, nodeToSplit->keys.end());
        // resizes full node so that it only contains the first half
        nodeToSplit->keys.resize(mid);

        newNode->children.assign(nodeToSplit->children.begin() + mid + 1, nodeToSplit->children.end());
        nodeToSplit->children.resize(mid + 1);

        parent->keys.insert(parent->keys.begin() + index, newKey);
        parent->children.insert(parent->children.begin() + index + 1, newNode);
    }
}

BTreeOrderBook::PriceLevel* BTreeOrderBook::find_price_level(BTreeNode* root, double price) const {
    BTreeNode* current = root;
    while (current != nullptr) {
        size_t i = 0;
        while (i < current->keys.size() && price > current->keys[i].price) {
            i++;
        }
        if (current->is_leaf) {
            if (i < current->keys.size() && price == current->keys[i].price) {
                return &current->keys[i];
            }
            // if leaf searched and price not found, does not exist
            else {
                return nullptr;
            }
        }

        current = current->children[i];
    }
    // if root is nullptr
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

void BTreeOrderBook::collect_levels(BTreeNode* node, std::vector<Level>& levels, size_t& count, size_t max_levels, bool reverse) const {
    if (count >= max_levels || node == nullptr) {
        return;
    }
    if (node->is_leaf) {
        // if reverse, add prices backwards (looking for highest price)
        if (reverse) {
            for (int i = int(node->keys.size()) - 1; i >= 0 && count < max_levels; --i) {
                PriceLevel &priceLvl = node->keys[i];
                if (!priceLvl.orders.empty()) {
                    double qty = 0.0;
                    for (const auto &order : priceLvl.orders) {
                        qty += order->get_quantity();
                    }
                    levels.emplace_back(priceLvl.price, qty, priceLvl.orders.size());
                    count++;
                }
            }
        }
        // if not reverse, add them forwards (looking for lowest price)
        else {
            for (int i = int(node->keys.size()) - 1; i >= 0 && count < max_levels; --i) {
                PriceLevel &priceLvl = node->keys[i];
                if (!priceLvl.orders.empty()) {
                    double qty = 0.0;
                    for (const auto &order : priceLvl.orders) {
                        qty += order->get_quantity();
                    }
                    levels.emplace_back(priceLvl.price, qty, priceLvl.orders.size());
                    count++;
                }
            }
        }
    }
    // if not a leaf, recurse down to leaf nodes
    else {
        int size = int(node->keys.size());
        if (reverse) {
            for (int i = size; i >= 0 && count < max_levels; --i) {
                collect_levels(node->children[i], levels, count, max_levels, reverse);
            }
        }
        else {
            for (int i = 0; i <= size && count < max_levels; ++i) {
                collect_levels(node->children[i], levels, count, max_levels, reverse);
            }
        }
    }
}

} // namespace order_matching