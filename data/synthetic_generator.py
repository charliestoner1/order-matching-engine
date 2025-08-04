#!/usr/bin/env python3
"""
Synthetic Order Generator for Order Matching Engine
Generates realistic order flow data for testing
"""

import csv
import random
import time
from datetime import datetime, timedelta
import numpy as np
import argparse
import os

class OrderGenerator:
    def __init__(self, symbol, base_price=100.0, volatility=0.02):
        self.symbol = symbol
        self.base_price = base_price
        self.volatility = volatility
        self.order_id = 1
        self.current_price = base_price

    def generate_order(self, timestamp):
        """Generate a single order with realistic characteristics"""
        # Random walk for price
        price_change = np.random.normal(0, self.volatility)
        self.current_price *= (1 + price_change)

        # Determine side with slight bias towards equilibrium
        if self.current_price > self.base_price * 1.05:
            side = 'SELL' if random.random() < 0.7 else 'BUY'
        elif self.current_price < self.base_price * 0.95:
            side = 'BUY' if random.random() < 0.7 else 'SELL'
        else:
            side = 'BUY' if random.random() < 0.5 else 'SELL'

        # Price around current market price
        if side == 'BUY':
            # Buy orders slightly below market
            price = self.current_price * random.uniform(0.995, 1.0)
        else:
            # Sell orders slightly above market
            price = self.current_price * random.uniform(1.0, 1.005)

        # Quantity with power law distribution (more small orders)
        quantity = int(np.random.pareto(1.5) * 100) + 1
        quantity = min(quantity, 10000)  # Cap at 10000

        order = {
            'order_id': self.order_id,
            'timestamp': int(timestamp * 1000000),  # microseconds
            'symbol': self.symbol,
            'side': side,
            'price': round(price, 2),
            'quantity': quantity
        }

        self.order_id += 1
        return order

def generate_burst_orders(generator, timestamp, num_orders, burst_side=None):
    """Generate a burst of orders (simulating news event or large trader)"""
    orders = []

    if burst_side is None:
        burst_side = random.choice(['BUY', 'SELL'])

    # Tight price range for burst
    if burst_side == 'BUY':
        price_range = (0.998, 1.002)
    else:
        price_range = (0.998, 1.002)

    for _ in range(num_orders):
        order = generator.generate_order(timestamp)
        order['side'] = burst_side
        # Adjust price to be more aggressive
        order['price'] = round(generator.current_price * random.uniform(*price_range), 2)
        orders.append(order)
        timestamp += random.uniform(0.0001, 0.001)  # Rapid succession

    return orders

def generate_order_file(filename, num_orders=100000, symbols=['AAPL', 'GOOGL', 'MSFT'],
                        duration_hours=1):
    """Generate a CSV file with synthetic orders"""

    print(f"Generating {num_orders} orders for symbols: {symbols}")

    generators = {}
    base_prices = {'AAPL': 150.0, 'GOOGL': 2800.0, 'MSFT': 400.0,
                   'TSLA': 200.0, 'AMZN': 180.0}

    for symbol in symbols:
        price = base_prices.get(symbol, 100.0)
        generators[symbol] = OrderGenerator(symbol, price)

    all_orders = []

    # Generate orders over time period
    start_time = time.time()
    end_time = start_time + (duration_hours * 3600)
    time_step = (end_time - start_time) / num_orders

    current_time = start_time
    orders_generated = 0

    while orders_generated < num_orders:
        # Choose random symbol
        symbol = random.choice(symbols)
        generator = generators[symbol]

        # 5% chance of burst activity
        if random.random() < 0.05 and orders_generated < num_orders - 50:
            burst_size = random.randint(10, 50)
            burst_orders = generate_burst_orders(generator, current_time, burst_size)
            all_orders.extend(burst_orders)
            orders_generated += len(burst_orders)
            current_time += time_step * len(burst_orders)
        else:
            # Normal order
            order = generator.generate_order(current_time)
            all_orders.append(order)
            orders_generated += 1
            current_time += time_step + random.uniform(-time_step/2, time_step/2)

        if orders_generated % 10000 == 0:
            print(f"  Generated {orders_generated} orders...")

    # Sort by timestamp
    all_orders.sort(key=lambda x: x['timestamp'])

    # Write to CSV
    with open(filename, 'w', newline='') as csvfile:
        fieldnames = ['order_id', 'timestamp', 'symbol', 'side', 'price', 'quantity']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(all_orders)

    print(f"Generated {len(all_orders)} orders")
    print(f"Output saved to: {filename}")

    # Generate summary statistics
    print("\nOrder Statistics:")
    for symbol in symbols:
        symbol_orders = [o for o in all_orders if o['symbol'] == symbol]
        if symbol_orders:
            buy_orders = [o for o in symbol_orders if o['side'] == 'BUY']
            sell_orders = [o for o in symbol_orders if o['side'] == 'SELL']
            avg_price = np.mean([o['price'] for o in symbol_orders])
            avg_qty = np.mean([o['quantity'] for o in symbol_orders])

            print(f"\n{symbol}:")
            print(f"  Total orders: {len(symbol_orders)}")
            print(f"  Buy orders: {len(buy_orders)}")
            print(f"  Sell orders: {len(sell_orders)}")
            print(f"  Avg price: ${avg_price:.2f}")
            print(f"  Avg quantity: {avg_qty:.0f}")

def main():
    parser = argparse.ArgumentParser(description='Generate synthetic order data')
    parser.add_argument('-o', '--output', default='data/sample_orders.csv',
                        help='Output CSV file path')
    parser.add_argument('-n', '--num-orders', type=int, default=100000,
                        help='Number of orders to generate')
    parser.add_argument('-s', '--symbols', nargs='+',
                        default=['AAPL', 'GOOGL', 'MSFT'],
                        help='Stock symbols to generate orders for')
    parser.add_argument('-d', '--duration', type=float, default=1.0,
                        help='Duration of order flow in hours')

    args = parser.parse_args()

    # Create data directory if it doesn't exist
    os.makedirs(os.path.dirname(args.output), exist_ok=True)

    generate_order_file(args.output, args.num_orders, args.symbols, args.duration)

if __name__ == '__main__':
    main()