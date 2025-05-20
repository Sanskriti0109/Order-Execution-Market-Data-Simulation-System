#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>
#include "order.h"
#include "C:\Users\sansk\goquant_assignment\include\deribit_api.h"
#include <time.h>
char* api_request(const char* url, const char* post_data, const char* access_token);


// Callback functions
static OrderCallback order_callback = NULL;
static OrderBookCallback orderbook_callback = NULL;
static PositionCallback position_callback = NULL;

// Register callbacks
void register_order_callback(OrderCallback callback) {
    order_callback = callback;
}

void register_orderbook_callback(OrderBookCallback callback) {
    orderbook_callback = callback;
}

void register_position_callback(PositionCallback callback) {
    position_callback = callback;
}

// Helper function to parse order from JSON
static void parse_order_from_json(cJSON* json_order, Order* order) {
    cJSON* order_id = cJSON_GetObjectItemCaseSensitive(json_order, "order_id");
    cJSON* instrument = cJSON_GetObjectItemCaseSensitive(json_order, "instrument_name");
    cJSON* price = cJSON_GetObjectItemCaseSensitive(json_order, "price");
    cJSON* amount = cJSON_GetObjectItemCaseSensitive(json_order, "amount");
    cJSON* direction = cJSON_GetObjectItemCaseSensitive(json_order, "direction");
    cJSON* order_type = cJSON_GetObjectItemCaseSensitive(json_order, "order_type");
    cJSON* order_state = cJSON_GetObjectItemCaseSensitive(json_order, "order_state");
    cJSON* created_timestamp = cJSON_GetObjectItemCaseSensitive(json_order, "creation_timestamp");
    cJSON* last_update_timestamp = cJSON_GetObjectItemCaseSensitive(json_order, "last_update_timestamp");

    if (cJSON_IsString(order_id) && order_id->valuestring) {
        strncpy(order->order_id, order_id->valuestring, sizeof(order->order_id) - 1);
    }
    
    if (cJSON_IsString(instrument) && instrument->valuestring) {
        strncpy(order->instrument_name, instrument->valuestring, sizeof(order->instrument_name) - 1);
    }
    
    if (cJSON_IsNumber(price)) {
        order->price = price->valuedouble;
    }
    
    if (cJSON_IsNumber(amount)) {
        order->amount = amount->valuedouble;
    }
    
    if (cJSON_IsString(direction) && direction->valuestring) {
        if (strcmp(direction->valuestring, "buy") == 0) {
            order->side = ORDER_SIDE_BUY;
        } else {
            order->side = ORDER_SIDE_SELL;
        }
    }
    
    if (cJSON_IsString(order_type) && order_type->valuestring) {
        if (strcmp(order_type->valuestring, "limit") == 0) {
            order->type = ORDER_TYPE_LIMIT;
        } else if (strcmp(order_type->valuestring, "market") == 0) {
            order->type = ORDER_TYPE_MARKET;
        } else if (strcmp(order_type->valuestring, "stop_limit") == 0) {
            order->type = ORDER_TYPE_STOP_LIMIT;
        } else if (strcmp(order_type->valuestring, "stop_market") == 0) {
            order->type = ORDER_TYPE_STOP_MARKET;
        }
    }
    
    if (cJSON_IsString(order_state) && order_state->valuestring) {
        if (strcmp(order_state->valuestring, "open") == 0) {
            order->status = ORDER_STATUS_OPEN;
        } else if (strcmp(order_state->valuestring, "filled") == 0) {
            order->status = ORDER_STATUS_FILLED;
        } else if (strcmp(order_state->valuestring, "rejected") == 0) {
            order->status = ORDER_STATUS_REJECTED;
        } else if (strcmp(order_state->valuestring, "cancelled") == 0) {
            order->status = ORDER_STATUS_CANCELLED;
        } else if (strcmp(order_state->valuestring, "untriggered") == 0) {
            order->status = ORDER_STATUS_UNTRIGGERED;
        }
    }
    
    // Convert timestamps to readable format
    if (cJSON_IsNumber(created_timestamp)) {
        time_t timestamp = (time_t)(created_timestamp->valuedouble / 1000);
        struct tm* timeinfo = gmtime(&timestamp);
        strftime(order->created_at, sizeof(order->created_at), "%Y-%m-%d %H:%M:%S", timeinfo);
    }
    
    if (cJSON_IsNumber(last_update_timestamp)) {
        time_t timestamp = (time_t)(last_update_timestamp->valuedouble / 1000);
        struct tm* timeinfo = gmtime(&timestamp);
        strftime(order->last_update, sizeof(order->last_update), "%Y-%m-%d %H:%M:%S", timeinfo);
    }
    
    // Trigger callback if registered
    if (order_callback) {
        order_callback(order);
    }
}

// Helper function to parse orderbook from JSON
static void parse_orderbook_from_json(cJSON* json_orderbook, OrderBook* orderbook) {
    cJSON* bids = cJSON_GetObjectItemCaseSensitive(json_orderbook, "bids");
    cJSON* asks = cJSON_GetObjectItemCaseSensitive(json_orderbook, "asks");
    cJSON* timestamp = cJSON_GetObjectItemCaseSensitive(json_orderbook, "timestamp");
    
    orderbook->bids_count = 0;
    orderbook->asks_count = 0;
    
    if (cJSON_IsArray(bids)) {
        orderbook->bids_count = cJSON_GetArraySize(bids);
        orderbook->bids = malloc(orderbook->bids_count * sizeof(OrderBookEntry));
        
        for (int i = 0; i < orderbook->bids_count; i++) {
            cJSON* bid = cJSON_GetArrayItem(bids, i);
            if (cJSON_IsArray(bid) && cJSON_GetArraySize(bid) >= 2) {
                cJSON* price = cJSON_GetArrayItem(bid, 0);
                cJSON* amount = cJSON_GetArrayItem(bid, 1);
                
                if (cJSON_IsNumber(price) && cJSON_IsNumber(amount)) {
                    orderbook->bids[i].price = price->valuedouble;
                    orderbook->bids[i].amount = amount->valuedouble;
                }
            }
        }
    }
    
    if (cJSON_IsArray(asks)) {
        orderbook->asks_count = cJSON_GetArraySize(asks);
        orderbook->asks = malloc(orderbook->asks_count * sizeof(OrderBookEntry));
        
        for (int i = 0; i < orderbook->asks_count; i++) {
            cJSON* ask = cJSON_GetArrayItem(asks, i);
            if (cJSON_IsArray(ask) && cJSON_GetArraySize(ask) >= 2) {
                cJSON* price = cJSON_GetArrayItem(ask, 0);
                cJSON* amount = cJSON_GetArrayItem(ask, 1);
                
                if (cJSON_IsNumber(price) && cJSON_IsNumber(amount)) {
                    orderbook->asks[i].price = price->valuedouble;
                    orderbook->asks[i].amount = amount->valuedouble;
                }
            }
        }
    }
    
    if (cJSON_IsNumber(timestamp)) {
        time_t ts = (time_t)(timestamp->valuedouble / 1000);
        struct tm* timeinfo = gmtime(&ts);
        strftime(orderbook->timestamp, sizeof(orderbook->timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    }
    
    // Trigger callback if registered
    if (orderbook_callback) {
        orderbook_callback(orderbook);
    }
}

// Helper function to parse position from JSON
static void parse_position_from_json(cJSON* json_position, Position* position) {
    cJSON* instrument = cJSON_GetObjectItemCaseSensitive(json_position, "instrument_name");
    cJSON* size = cJSON_GetObjectItemCaseSensitive(json_position, "size");
    cJSON* entry_price = cJSON_GetObjectItemCaseSensitive(json_position, "average_price");
    cJSON* mark_price = cJSON_GetObjectItemCaseSensitive(json_position, "mark_price");
    cJSON* unrealized_pnl = cJSON_GetObjectItemCaseSensitive(json_position, "floating_profit_loss");
    cJSON* realized_pnl = cJSON_GetObjectItemCaseSensitive(json_position, "realized_profit_loss");
    cJSON* last_update_timestamp = cJSON_GetObjectItemCaseSensitive(json_position, "last_update_timestamp");
    
    if (cJSON_IsString(instrument) && instrument->valuestring) {
        strncpy(position->instrument_name, instrument->valuestring, sizeof(position->instrument_name) - 1);
    }
    
    if (cJSON_IsNumber(size)) {
        position->size = size->valuedouble;
    }
    
    if (cJSON_IsNumber(entry_price)) {
        position->entry_price = entry_price->valuedouble;
    }
    
    if (cJSON_IsNumber(mark_price)) {
        position->mark_price = mark_price->valuedouble;
    }
    
    if (cJSON_IsNumber(unrealized_pnl)) {
        position->unrealized_pnl = unrealized_pnl->valuedouble;
    }
    
    if (cJSON_IsNumber(realized_pnl)) {
        position->realized_pnl = realized_pnl->valuedouble;
    }
    
    if (cJSON_IsNumber(last_update_timestamp)) {
        time_t timestamp = (time_t)(last_update_timestamp->valuedouble / 1000);
        struct tm* timeinfo = gmtime(&timestamp);
        strftime(position->timestamp, sizeof(position->timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    }
    
    // Trigger callback if registered
    if (position_callback) {
        position_callback(position);
    }
}

// Place a new order
bool place_order(const char* symbol, const char* price, const char* amount, const char* access_token, Order* out_order) {
    char url[256];
    char post_data[512];
    
    snprintf(url, sizeof(url), "https://www.deribit.com/api/v2/private/buy");
    snprintf(post_data, sizeof(post_data), 
        "{\"instrument_name\":\"%s\",\"amount\":%s,\"price\":%s,\"type\":\"limit\",\"post_only\":true}", 
        symbol, amount, price);
    
    char* response = api_request(url, post_data, access_token);
    if (!response) {
        return false;
    }
    
    cJSON* json = cJSON_Parse(response);
    if (!json) {
        free(response);
        return false;
    }
    
    bool success = false;
    cJSON* result = cJSON_GetObjectItemCaseSensitive(json, "result");
    if (cJSON_IsObject(result) && out_order) {
        parse_order_from_json(result, out_order);
        success = true;
    }
    
    cJSON_Delete(json);
    free(response);
    return success;
}

// Cancel an existing order
bool cancel_order(const char* order_id, const char* access_token) {
    char url[256];
    char post_data[512];
    
    snprintf(url, sizeof(url), "https://www.deribit.com/api/v2/private/cancel");
    snprintf(post_data, sizeof(post_data), "{\"order_id\":\"%s\"}", order_id);
    
    char* response = api_request(url, post_data, access_token);
    if (!response) {
        return false;
    }
    
    cJSON* json = cJSON_Parse(response);
    if (!json) {
        free(response);
        return false;
    }
    
    bool success = false;
    cJSON* result = cJSON_GetObjectItemCaseSensitive(json, "result");
    if (cJSON_IsObject(result)) {
        success = true;
    }
    
    cJSON_Delete(json);
    free(response);
    return success;
}

// Modify an existing order
bool modify_order(const char* order_id, const char* new_price, const char* new_amount, const char* access_token, Order* out_order) {
    char url[256];
    char post_data[512];
    
    snprintf(url, sizeof(url), "https://www.deribit.com/api/v2/private/edit");
    snprintf(post_data, sizeof(post_data), 
        "{\"order_id\":\"%s\",\"amount\":%s,\"price\":%s}", 
        order_id, new_amount, new_price);
    
    char* response = api_request(url, post_data, access_token);
    if (!response) {
        return false;
    }
    
    cJSON* json = cJSON_Parse(response);
    if (!json) {
        free(response);
        return false;
    }
    
    bool success = false;
    cJSON* result = cJSON_GetObjectItemCaseSensitive(json, "result");
    if (cJSON_IsObject(result) && out_order) {
        parse_order_from_json(result, out_order);
        success = true;
    }
    
    cJSON_Delete(json);
    free(response);
    return success;
}

// Get open orders
bool get_open_orders(const char* symbol, const char* access_token, Order** out_orders, int* out_count) {
    char url[256];
    char post_data[512];
    
    snprintf(url, sizeof(url), "https://www.deribit.com/api/v2/private/get_open_orders_by_instrument");
    snprintf(post_data, sizeof(post_data), "{\"instrument_name\":\"%s\"}", symbol);
    
    char* response = api_request(url, post_data, access_token);
    if (!response) {
        return false;
    }
    
    cJSON* json = cJSON_Parse(response);
    if (!json) {
        free(response);
        return false;
    }
    
    bool success = false;
    cJSON* result = cJSON_GetObjectItemCaseSensitive(json, "result");
    if (cJSON_IsArray(result)) {
        int count = cJSON_GetArraySize(result);
        *out_count = count;
        
        if (count > 0) {
            *out_orders = malloc(count * sizeof(Order));
            for (int i = 0; i < count; i++) {
                cJSON* order = cJSON_GetArrayItem(result, i);
                parse_order_from_json(order, &(*out_orders)[i]);
            }
        } else {
            *out_orders = NULL;
        }
        
        success = true;
    }
    
    cJSON_Delete(json);
    free(response);
    return success;
}

// Get order history
bool get_order_history(const char* symbol, const char* access_token, Order** out_orders, int* out_count) {
    char url[256];
    char post_data[512];
    
    snprintf(url, sizeof(url), "https://www.deribit.com/api/v2/private/get_order_history_by_instrument");
    snprintf(post_data, sizeof(post_data), 
        "{\"instrument_name\":\"%s\",\"count\":20,\"include_old\":true}", 
        symbol);
    
    char* response = api_request(url, post_data, access_token);
    if (!response) {
        return false;
    }
    
    cJSON* json = cJSON_Parse(response);
    if (!json) {
        free(response);
        return false;
    }
    
    bool success = false;
    cJSON* result = cJSON_GetObjectItemCaseSensitive(json, "result");
    if (cJSON_IsArray(result)) {
        int count = cJSON_GetArraySize(result);
        *out_count = count;
        
        if (count > 0) {
            *out_orders = malloc(count * sizeof(Order));
            for (int i = 0; i < count; i++) {
                cJSON* order = cJSON_GetArrayItem(result, i);
                parse_order_from_json(order, &(*out_orders)[i]);
            }
        } else {
            *out_orders = NULL;
        }
        
        success = true;
    }
    
    cJSON_Delete(json);
    free(response);
    return success;
}

//5
// Simple helpers
void get_orderbook_simple(const char* symbol) {
    char access_token[1024] = {0};
    
    // First get access token using default credentials
    if (!get_access_token(NULL, NULL, access_token)) {
        printf("Failed to get access token\n");
        return;
    }
    
    OrderBook orderbook = {0};
    if (!get_orderbook(symbol, 5, access_token, &orderbook)) {
        printf("Failed to get orderbook\n");
        return;
    }
    
    printf("Orderbook for %s:\n", symbol);
    printf("Bids:\n");
    for (int i = 0; i < orderbook.bids_count; i++) {
        printf("%.2f @ %.6f\n", orderbook.bids[i].price, orderbook.bids[i].amount);
    }
    
    printf("Asks:\n");
    for (int i = 0; i < orderbook.asks_count; i++) {
        printf("%.2f @ %.6f\n", orderbook.asks[i].price, orderbook.asks[i].amount);
    }
    
    // Free memory
    if (orderbook.bids) free(orderbook.bids);
    if (orderbook.asks) free(orderbook.asks);
}

void get_positions_simple(const char* access_token) {
    Position* positions = NULL;
    int count = 0;
    
    if (!get_positions("BTC", "future", access_token, &positions, &count)) {
        printf("Failed to get positions\n");
        return;
    }
    
    printf("Found %d positions:\n", count);
    for (int i = 0; i < count; i++) {
        printf("Instrument: %s\n", positions[i].instrument_name);
        printf("Size: %.6f\n", positions[i].size);
        printf("Entry Price: %.2f\n", positions[i].entry_price);
        printf("Mark Price: %.2f\n", positions[i].mark_price);
        printf("Unrealized PnL: %.6f\n", positions[i].unrealized_pnl);
    }
    
    // Free memory
    if (positions) free(positions);
}

// Helper function to make API requests through deribit_api.h
char* api_request(const char* url, const char* post_data, const char* access_token) {
    // This function should use the appropriate function from deribit_api.h
    // This is a placeholder - you need to implement it according to your deribit_api.h
    // For example, it might call something like:
    return perform_request(url, post_data, access_token);
}