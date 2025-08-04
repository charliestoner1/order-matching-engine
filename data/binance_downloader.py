# binance_downloader.py
import requests
import json
import csv
from datetime import datetime
import time

class BinanceDownloader:
    def __init__(self):
        self.base_url = "https://api.binance.com/api/v3"

    def download_orderbook_snapshot(self, symbol="BTCUSDT", limit=1000):
        """Download current order book snapshot"""
        url = f"{self.base_url}/depth"
        params = {"symbol": symbol, "limit": limit}

        response = requests.get(url, params=params)
        if response.status_code == 200:
            return response.json()
        return None

    def download_recent_trades(self, symbol="BTCUSDT", limit=1000):
        """Download recent trades"""
        url = f"{self.base_url}/trades"
        params = {"symbol": symbol, "limit": limit}

        response = requests.get(url, params=params)
        if response.status_code == 200:
            return response.json()
        return None

    def convert_to_order_format(self, orderbook_data, symbol):
        """Convert Binance data to order format"""
        orders = []
        order_id = 1

        # Process bids (buy orders)
        for bid in orderbook_data['bids']:
            price = float(bid[0])
            quantity = float(bid[1])
            orders.append({
                'order_id': order_id,
                'timestamp': int(time.time() * 1000000),
                'symbol': symbol,
                'side': 'BUY',
                'price': price,
                'quantity': quantity
            })
            order_id += 1

        # Process asks (sell orders)
        for ask in orderbook_data['asks']:
            price = float(ask[0])
            quantity = float(ask[1])
            orders.append({
                'order_id': order_id,
                'timestamp': int(time.time() * 1000000),
                'symbol': symbol,
                'side': 'SELL',
                'price': price,
                'quantity': quantity
            })
            order_id += 1

        return orders

    def save_to_csv(self, orders, filename="binance_orders.csv"):
        """Save orders to CSV format"""
        with open(filename, 'w', newline='') as csvfile:
            fieldnames = ['order_id', 'timestamp', 'symbol', 'side', 'price', 'quantity']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(orders)
        print(f"Saved {len(orders)} orders to {filename}")

def main():
    downloader = BinanceDownloader()

    # Download order book for multiple symbols
    symbols = ['BTCUSDT', 'ETHUSDT', 'BNBUSDT']
    all_orders = []

    for symbol in symbols:
        print(f"Downloading {symbol} order book...")
        orderbook = downloader.download_orderbook_snapshot(symbol, limit=500)

        if orderbook:
            orders = downloader.convert_to_order_format(orderbook, symbol)
            all_orders.extend(orders)
            print(f"  Got {len(orders)} orders")
            time.sleep(0.5)  # Rate limiting

    # Save to CSV
    downloader.save_to_csv(all_orders, "data/binance_orders.csv")

    # Also download some trade history for replay
    print("\nDownloading recent trades...")
    for symbol in symbols:
        trades = downloader.download_recent_trades(symbol, limit=500)
        if trades:
            # Save trades for replay simulation
            with open(f"data/{symbol}_trades.json", 'w') as f:
                json.dump(trades, f)
            print(f"  Saved {len(trades)} trades for {symbol}")
            time.sleep(0.5)

if __name__ == "__main__":
    main()