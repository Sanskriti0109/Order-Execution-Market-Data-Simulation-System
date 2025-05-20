#ifndef ORDERS_H
#define ORDERS_H

#include <stdbool.h>  // Required for bool
#include <stddef.h>   // Required for size_t

#ifdef __cplusplus
extern "C" {
#endif

// Order types
typedef enum {
    ORDER_TYPE_LIMIT,
    ORDER_TYPE_MARKET,
    ORDER_TYPE_STOP_LIMIT,
    ORDER_TYPE_STOP_MARKET
} OrderType;

// Order sides
typedef enum {
    ORDER_SIDE_BUY,
    ORDER_SIDE_SELL
} OrderSide;

// Order status
typedef enum {
    ORDER_STATUS_OPEN,
    ORDER_STATUS_FILLED,
    ORDER_STATUS_REJECTED,
    ORDER_STATUS_CANCELLED,
    ORDER_STATUS_UNTRIGGERED
} OrderStatus;

// Order structure
typedef struct {
    char order_id[64];
    char instrument_name[32];
    OrderType type;
    OrderSide side;
    double price;
    double amount;
    OrderStatus status;
    char created_at[32];
    char last_update[32];
} Order;

// Order book structure
typedef struct {
    double price;
    double amount;
} OrderBookEntry;

typedef struct {
    OrderBookEntry* bids;
    int bids_count;
    OrderBookEntry* asks;
    int asks_count;
    char timestamp[32];
} OrderBook;

// Position structure
typedef struct {
    char instrument_name[32];
    double size;
    double entry_price;
    double mark_price;
    double unrealized_pnl;
    double realized_pnl;
    char timestamp[32];
} Position;

// 🛠️ Updated Function Signatures (match main.c usage)
bool place_order(const char* symbol, const char* price, const char* amount, const char* access_token, Order* out_order);
bool cancel_order(const char* order_id, const char* access_token);
bool modify_order(const char* order_id, const char* new_price, const char* new_amount, const char* access_token, Order* out_order);

bool get_open_orders(const char* symbol, const char* access_token, Order** out_orders, int* out_count);
bool get_order_history(const char* symbol, const char* access_token, Order** out_orders, int* out_count);

// Optional helpers
void get_orderbook_simple(const char* symbol);
void get_positions_simple(const char* access_token);

// Callback function types
typedef void (*OrderCallback)(const Order* order);
typedef void (*OrderBookCallback)(const OrderBook* orderbook);
typedef void (*PositionCallback)(const Position* position);

// Register callbacks
void register_order_callback(OrderCallback callback);
void register_orderbook_callback(OrderBookCallback callback);
void register_position_callback(PositionCallback callback);

#ifdef __cplusplus
}
#endif

#endif // ORDERS_H
