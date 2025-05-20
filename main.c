#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "C:\Users\sansk\goquant_assignment\include\deribit_api.h"
#include "C:\Users\sansk\goquant_assignment\include\order.h"

#define CLIENT_ID "-8oxD0z5"
#define CLIENT_SECRET "yrOhwkelwTUSLS57hW_0UWg-J0cnD9S_PJxBHWXkPoo"
#define TOKEN_SIZE 1024

void print_order(const Order* order) {
    printf("Order ID: %s\n", order->order_id);
    printf("Instrument: %s\n", order->instrument_name);
    printf("Price: %.2f\n", order->price);
    printf("Amount: %.6f\n", order->amount);
    printf("Side: %s\n", order->side == ORDER_SIDE_BUY ? "Buy" : "Sell");
    printf("Type: ");
    switch (order->type) {
        case ORDER_TYPE_LIMIT: printf("Limit\n"); break;
        case ORDER_TYPE_MARKET: printf("Market\n"); break;
        case ORDER_TYPE_STOP_LIMIT: printf("Stop Limit\n"); break;
        case ORDER_TYPE_STOP_MARKET: printf("Stop Market\n"); break;
        default: printf("Unknown\n"); break;
    }
    printf("Status: ");
    switch (order->status) {
        case ORDER_STATUS_OPEN: printf("Open\n"); break;
        case ORDER_STATUS_FILLED: printf("Filled\n"); break;
        case ORDER_STATUS_REJECTED: printf("Rejected\n"); break;
        case ORDER_STATUS_CANCELLED: printf("Cancelled\n"); break;
        case ORDER_STATUS_UNTRIGGERED: printf("Untriggered\n"); break;
        default: printf("Unknown\n"); break;
    }
    printf("Created at: %s\n", order->created_at);
    printf("Last update: %s\n", order->last_update);
    printf("------------------------------\n");
}

void print_position(const Position* position) {
    printf("Instrument: %s\n", position->instrument_name);
    printf("Size: %.6f\n", position->size);
    printf("Entry Price: %.2f\n", position->entry_price);
    printf("Mark Price: %.2f\n", position->mark_price);
    printf("Unrealized PnL: %.6f\n", position->unrealized_pnl);
    printf("Realized PnL: %.6f\n", position->realized_pnl);
    printf("Timestamp: %s\n", position->timestamp);
    printf("------------------------------\n");
}

void print_orderbook(const OrderBook* orderbook) {
    printf("Orderbook Timestamp: %s\n", orderbook->timestamp);
    printf("\nBids:\nPrice\t\tAmount\n");
    for (int i = 0; i < orderbook->bids_count && i < 5; i++) {
        printf("%.2f\t\t%.6f\n", orderbook->bids[i].price, orderbook->bids[i].amount);
    }
    printf("\nAsks:\nPrice\t\tAmount\n");
    for (int i = 0; i < orderbook->asks_count && i < 5; i++) {
        printf("%.2f\t\t%.6f\n", orderbook->asks[i].price, orderbook->asks[i].amount);
    }
    printf("------------------------------\n");
}

void print_error() {
    DeribitError error = get_last_error();
    printf("Error code: %d\n", error.code);
    printf("Error message: %s\n", error.message);
}

int main() {
    char access_token[TOKEN_SIZE] = {0};
    const char* instrument_name = "BTC-PERPETUAL";

    // Step 1: Get access token
    printf("Getting access token...\n");
    if (!get_access_token(CLIENT_ID, CLIENT_SECRET, access_token)) {
        printf("Failed to retrieve access token.\n");
        print_error();
        return 1;
    }
    printf("Successfully retrieved access token.\n");

    // Step 2: Get orderbook
    printf("\nGetting orderbook for %s...\n", instrument_name);
    OrderBook orderbook = {0};
    if (!get_orderbook(instrument_name, 10, access_token, &orderbook)) {
        printf("Failed to get orderbook.\n");
        print_error();
    } else {
        print_orderbook(&orderbook);
        // Free dynamically allocated bids and asks if allocated
        if (orderbook.bids) free(orderbook.bids);
        if (orderbook.asks) free(orderbook.asks);
    }

    // Step 3: Get positions
    printf("\nGetting current positions...\n");
    Position* positions = NULL;
    int positions_count = 0;
    if (!get_positions("BTC", "future", access_token, &positions, &positions_count)) {
        printf("Failed to get positions.\n");
        print_error();
    } else {
        printf("Found %d positions:\n", positions_count);
        for (int i = 0; i < positions_count; i++) {
            print_position(&positions[i]);
        }
        if (positions) free(positions);
    }

    // Step 4: Get open orders
    printf("\nGetting open orders...\n");
    Order* orders = NULL;
    int orders_count = 0;
    if (!get_open_orders(instrument_name, access_token, &orders, &orders_count)) {
        printf("Failed to get open orders.\n");
        print_error();
    } else {
        printf("Found %d open orders:\n", orders_count);
        for (int i = 0; i < orders_count; i++) {
            print_order(&orders[i]);
        }
        if (orders) free(orders);
    }

    // Step 5: Place a limit order
    printf("\nPlacing a limit order...\n");
    Order new_order = {0};
    double price_val = 25000.0;
    double amount_val = 0.01;
    char price_str[32], amount_str[32];
    snprintf(price_str, sizeof(price_str), "%.2f", price_val);
    snprintf(amount_str, sizeof(amount_str), "%.6f", amount_val);

    if (!place_order(instrument_name, price_str, amount_str, access_token, &new_order)) {
        printf("Failed to place order.\n");
        print_error();
    } else {
        printf("Order placed successfully:\n");
        print_order(&new_order);

        // Step 6: Modify the order
        printf("\nModifying the order...\n");
        Order modified_order = {0};
        snprintf(price_str, sizeof(price_str), "%.2f", price_val + 1000.0);
        if (!modify_order(new_order.order_id, price_str, amount_str, access_token, &modified_order)) {
            printf("Failed to modify order.\n");
            print_error();
        } else {
            printf("Order modified successfully:\n");
            print_order(&modified_order);
        }

        // Step 7: Cancel the order
        printf("\nCancelling the order...\n");
        if (!cancel_order(new_order.order_id, access_token)) {
            printf("Failed to cancel order.\n");
            print_error();
        } else {
            printf("Order cancelled successfully.\n");
        }
    }

    // Step 8: Get order history
    printf("\nGetting order history...\n");
    Order* history_orders = NULL;
    int history_count = 0;
    if (!get_order_history(instrument_name, access_token, &history_orders, &history_count)) {
        printf("Failed to get order history.\n");
        print_error();
    } else {
        printf("Found %d orders in history:\n", history_count);
        for (int i = 0; i < history_count && i < 5; i++) {
            print_order(&history_orders[i]);
        }
        if (history_orders) free(history_orders);
    }

    printf("Program completed.\n");
    return 0;
}
