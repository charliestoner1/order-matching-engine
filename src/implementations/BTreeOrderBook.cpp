#include "BTreeOrderBook.h"

#include <algorithm>
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

    // insert into appropriate tree
    if (order->get_side() == BUY) {
        insert(buy_tree_root_, order->get_price(), order);
        ++bid_count_;
    } else {
        insert(sell_tree_root_, order->get_price(), order);
        ++ask_count_;
    }

    // track order location for fast cancellation
    order_location_[order->get_order_id()] = make_pair(order->get_side(), order->get_price());

    ++total_orders_;
    return true;
}

bool BTreeOrderBook::cancel_order(Order::OrderId order_id) {
    auto itr = order_location_.find(order_id);
    if (itr == order_location_.end()) {
        return false;
    }

    Side side = itr->second.first;
    double price = itr->second.second;

    PriceLevel* priceLvl = find_price_level(
        side == BUY ? buy_tree_root_ : sell_tree_root_, price);

    if (!priceLvl) {
        return false;
    }

    auto& orders = priceLvl->orders;
    for (auto it = orders.begin(); it != orders.end(); ++it) {
        if ((*it)->get_order_id() == order_id) {
            (*it)->cancel();
            orders.erase(it);
            order_location_.erase(order_id);

            if (side == BUY) {
                --bid_count_;
            } else {
                --ask_count_;
            }
            --total_orders_;

            // Clean up empty price levels if needed
            if (orders.empty()) {
                // Note: In a production system, you'd remove empty nodes
                // but for simplicity we'll leave them
            }

            return true;
        }
    }
    return false;
}

std::vector<Trade> BTreeOrderBook::match_orders() {
    std::vector<Trade> trades;
    trades.reserve(100);

    while (true) {
        // get best bid and ask prices
        double best_bid_price = get_best_bid();
        double best_ask_price = get_best_ask();

        // check if prices cross
        if (best_bid_price == 0 || best_ask_price == 0 || best_bid_price < best_ask_price) {
            break;
        }

        // get price levels
        PriceLevel* bid_level = find_price_level(buy_tree_root_, best_bid_price);
        PriceLevel* ask_level = find_price_level(sell_tree_root_, best_ask_price);

        if (!bid_level || !ask_level || bid_level->orders.empty() || ask_level->orders.empty()) {
            break;
        }

        // match orders at these levels
        auto& buy_order = bid_level->orders.front();
        auto& sell_order = ask_level->orders.front();

        // determine trade quantity
        double trade_qty = min(buy_order->get_remaining_quantity(), sell_order->get_remaining_quantity());

        // create trade - using ask price
        trades.emplace_back(
            generate_trade_id(),
            buy_order->get_order_id(),
            sell_order->get_order_id(),
            best_ask_price,
            trade_qty,
            symbol_
        );

        // update order quantities
        buy_order->set_remaining_quantity(buy_order->get_remaining_quantity() - trade_qty);
        sell_order->set_remaining_quantity(sell_order->get_remaining_quantity() - trade_qty);

        // remove filled orders
        if (buy_order->is_filled()) {
            bid_level->orders.pop_front();
            order_location_.erase(buy_order->get_order_id());
            --bid_count_;
            --total_orders_;
        }

        if (sell_order->is_filled()) {
            ask_level->orders.pop_front();
            order_location_.erase(sell_order->get_order_id());
            --ask_count_;
            --total_orders_;
        }

        // increment total trades
        ++total_trades_;
    }
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
    levels.reserve(max_levels);
    collect_levels(buy_tree_root_, levels, max_levels, true);
    return levels;
}

std::vector<OrderBook::Level> BTreeOrderBook::get_ask_levels(size_t max_levels) const {
    std::vector<Level> levels;
    levels.reserve(max_levels);
    collect_levels(sell_tree_root_, levels, max_levels, false);
    return levels;
}

// B-Tree helper methods
void BTreeOrderBook::insert(BTreeNode*& root, double price, std::shared_ptr<Order> order) {
    // Handle root split if needed
    if (root->keys.size() == max_keys_) {
        BTreeNode* newRoot = new BTreeNode();
        newRoot->is_leaf = false;
        newRoot->children.push_back(root);
        split_child(newRoot, 0);
        root = newRoot;
    }

    // Insert into non-full node
    BTreeNode* current = root;
    while (!current->is_leaf) {
        // Binary search for correct child
        size_t i = binary_search_price(current->keys, price);

        if (i < current->keys.size() && current->keys[i].price == price) {
            current->keys[i].orders.push_back(order);
            return;
        }

        if (current->children[i]->keys.size() == max_keys_) {
            split_child(current, i);
            if (price > current->keys[i].price) {
                i++;
            }
        }
        current = current->children[i];
    }

    // Insert into leaf
    int i = binary_search_price(current->keys, price);

    if (i < current->keys.size() && current->keys[i].price == price) {
        current->keys[i].orders.push_back(order);
    } else {
        PriceLevel newLevel(price);
        newLevel.orders.push_back(order);
        current->keys.insert(current->keys.begin() + i, newLevel);
    }
}

// Binary search helper for better cache performance
int BTreeOrderBook::binary_search_price(const std::vector<PriceLevel>& keys, double price) const {
    int left = 0;
    int right = keys.size();

    while (left < right) {
        int mid = left + (right - left) / 2;
        if (keys[mid].price < price) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }

    return left;
}


void BTreeOrderBook::split_child(BTreeNode* parent, int index) {
    BTreeNode* child = parent->children[index];
    int mid = child->keys.size() / 2;

    BTreeNode* newNode = new BTreeNode();
    newNode->is_leaf = child->is_leaf;

    // For leaf nodes, we need to keep the middle key in the original node
    // For internal nodes, the middle key moves up to the parent
    if (child->is_leaf) {
        // Copy right half to new node (excluding middle)
        newNode->keys.assign(child->keys.begin() + mid + 1, child->keys.end());

        // Keep left half including middle in original node
        child->keys.resize(mid + 1);

        // link newNode to leaf chain. Links are only for leaf nodes
        newNode->next = child->next;
        if (newNode->next != nullptr) {
            newNode->next->prev = newNode;
        }
        child->next = newNode;
        newNode->prev = child;

        // Insert a copy of the middle key into parent
        parent->keys.insert(parent->keys.begin() + index, child->keys[mid]);
    } else {
        // keys only store routing data when a non-leaf is splitting
        // Save middle key
        PriceLevel middleKey = child->keys[mid];

        // Copy right half to new node (excluding middle)
        newNode->keys.assign(child->keys.begin() + mid + 1, child->keys.end());
        child->keys.resize(mid);

        // Move children pointers
        newNode->children.assign(child->children.begin() + mid + 1, child->children.end());
        child->children.resize(mid + 1);

        // Insert middle key into parent
        parent->keys.insert(parent->keys.begin() + index, middleKey);
    }

    parent->children.insert(parent->children.begin() + index + 1, newNode);
}


BTreeOrderBook::PriceLevel* BTreeOrderBook::find_price_level(BTreeNode* root, double price) const {
    if (!root) return nullptr;

    BTreeNode* current = root;
    while (current) {
        size_t i = binary_search_price(current->keys, price);

        if (i < current->keys.size() && current->keys[i].price == price) {
            return &current->keys[i];
        }

        if (current->is_leaf) {
            return nullptr;
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

    // navigate to the appropriate leaf
    while (!current->is_leaf) {
        if (current->children.empty()) {
            break;
        }
        current = find_max ? current->children.back() : current->children.front();
    }

    // Find non-empty price level
    if (find_max) {
        for (int i = current->keys.size() - 1; i >= 0; --i) {
            if (!current->keys[i].orders.empty()) {
                return current->keys[i].price;
            }
        }
    } else {
        for (size_t i = 0; i < current->keys.size(); ++i) {
            if (!current->keys[i].orders.empty()) {
                return current->keys[i].price;
            }
        }
    }

    return 0.0;
}

void BTreeOrderBook::collect_levels(BTreeNode* node, std::vector<Level>& levels, size_t max_levels, bool reverse) const {
    size_t count = 0;
    if (node == nullptr) {
        return;
    }
    // find the necessary node based on which order
    BTreeNode* leaf = node;
    while (leaf != nullptr && !leaf->is_leaf) {
        if (reverse) {
            leaf = leaf->children.back();
        }
        else {
            leaf = leaf->children.front();
        }
    }
    // traverse the list if needed
    while (leaf != nullptr && count < max_levels) {
        // if backward then iterate backwards using prev pointers
        if (reverse) {
            for (int i = int(leaf->keys.size()) - 1; i >= 0 && count < max_levels; --i) {
                const PriceLevel& priceLvl = leaf->keys[i];
                if (!priceLvl.orders.empty()) {
                    double qty = 0.0;
                    for (const auto &order : priceLvl.orders) {
                        qty += order->get_remaining_quantity();
                    }
                    levels.emplace_back(priceLvl.price, qty, priceLvl.orders.size());
                    count++;
                }
            }
            leaf = leaf->prev;
        }
        // if forward simply iterate through the list forward using next pointers
        else {
            for (int i = 0; i < int(leaf->keys.size()) && count < max_levels; ++i) {
                const PriceLevel &priceLvl = leaf->keys[i];
                if (!priceLvl.orders.empty()) {
                    double qty = 0.0;
                    for (const auto &order : priceLvl.orders) {
                        qty += order->get_remaining_quantity();
                    }
                    levels.emplace_back(priceLvl.price, qty, priceLvl.orders.size());
                    count++;
                }
            }
            leaf = leaf->next;
        }
    }
}



} // namespace order_matching