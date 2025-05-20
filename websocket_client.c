#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "websocket_client.h"

// Simple implementation without actual network functionality
// Mock implementation for demonstration purposes

// Internal state variables
static WebSocketStatus current_status = WS_STATUS_DISCONNECTED;
static WebSocketConnectCallback connect_cb = NULL;
static WebSocketMessageCallback message_cb = NULL;
static WebSocketErrorCallback error_cb = NULL;
static WebSocketCloseCallback close_cb = NULL;
static char current_url[256] = {0};
static bool auto_reconnect = false;
static int max_retries = 5;
static int retry_delay_ms = 1000;
static int max_delay_ms = 30000;
static int subscription_count = 0;
static char subscriptions[32][128] = {0};  // Store up to 32 subscriptions

bool websocket_init() {
    printf("WebSocket client initialized\n");
    current_status = WS_STATUS_DISCONNECTED;
    subscription_count = 0;
    return true;
}

bool websocket_connect(const char* url, 
                      WebSocketConnectCallback connect_callback,
                      WebSocketMessageCallback message_callback,
                      WebSocketErrorCallback error_callback,
                      WebSocketCloseCallback close_callback) {
    if (!url) {
        return false;
    }

    // Store callbacks and URL
    connect_cb = connect_callback;
    message_cb = message_callback;
    error_cb = error_callback;
    close_cb = close_callback;
    strncpy(current_url, url, sizeof(current_url) - 1);
    
    // Simulate connection
    current_status = WS_STATUS_CONNECTING;
    printf("Connecting to: %s\n", url);
    
    // Simulate successful connection
    current_status = WS_STATUS_CONNECTED;
    if (connect_cb) {
        connect_cb(true, "Connected successfully");
    }
    
    return true;
}

bool websocket_send(const char* message) {
    if (current_status != WS_STATUS_CONNECTED) {
        printf("Cannot send: Not connected\n");
        return false;
    }
    
    printf("Sending text message: %s\n", message);
    return true;
}

bool websocket_send_binary(const unsigned char* data, size_t length) {
    if (current_status != WS_STATUS_CONNECTED) {
        printf("Cannot send binary: Not connected\n");
        return false;
    }
    
    printf("Sending binary message of length %zu\n", length);
    return true;
}

bool websocket_send_request(const char* method, const char* params, int request_id) {
    if (current_status != WS_STATUS_CONNECTED) {
        printf("Cannot send request: Not connected\n");
        return false;
    }
    
    // Format JSON-RPC request
    char request[1024];
    snprintf(request, sizeof(request), 
            "{\"jsonrpc\":\"2.0\",\"id\":%d,\"method\":\"%s\",\"params\":%s}",
            request_id, method, params);
    
    return websocket_send(request);
}

bool websocket_subscribe_with_params(const SubscriptionRequest* request, 
                                   WebSocketSubscriptionCallback callback) {
    if (!request || current_status != WS_STATUS_CONNECTED) {
        return false;
    }
    
    char channel[128] = {0};
    websocket_build_channel_name(channel, sizeof(channel), 
                               request->type, request->instrument_name,
                               request->interval, request->depth);
    
    // Check if already subscribed
    for (int i = 0; i < subscription_count; i++) {
        if (strcmp(subscriptions[i], channel) == 0) {
            printf("Already subscribed to: %s\n", channel);
            if (callback) {
                callback(channel, true);
            }
            return true;
        }
    }
    
    // Add to subscriptions
    if (subscription_count < 32) {
        strncpy(subscriptions[subscription_count], channel, sizeof(subscriptions[0]) - 1);
        subscription_count++;
        printf("Subscribed to: %s\n", channel);
        
        // Simulate subscription success
        if (callback) {
            callback(channel, true);
        }
        
        // Simulate receiving a message
        if (message_cb) {
            WebSocketMessage msg = {
                .type = WS_MESSAGE_TEXT,
                .data = "{\n  \"type\": \"snapshot\",\n  \"channel\": \"deribit_price_index.btc_usd\",\n  \"data\": {\n    \"timestamp\": 1590399365927,\n    \"price\": 25000.00,\n    \"index_name\": \"btc_usd\"\n  }\n}",
                .length = 162
            };
            strncpy(msg.channel, channel, sizeof(msg.channel) - 1);
            message_cb(&msg);
        }
        
        return true;
    }
    
    return false;
}

bool websocket_subscribe(const char* channel, WebSocketSubscriptionCallback callback) {
    if (!channel || current_status != WS_STATUS_CONNECTED) {
        return false;
    }
    
    // Simplified subscription
    SubscriptionRequest req = {0};
    req.type = SUBSCRIPTION_TICKER;  // Default type
    strncpy(req.instrument_name, channel, sizeof(req.instrument_name) - 1);
    
    return websocket_subscribe_with_params(&req, callback);
}

bool websocket_unsubscribe(const char* channel) {
    if (!channel) {
        return false;
    }
    
    // Find and remove subscription
    for (int i = 0; i < subscription_count; i++) {
        if (strcmp(subscriptions[i], channel) == 0) {
            // Remove by shifting remaining elements
            for (int j = i; j < subscription_count - 1; j++) {
                strcpy(subscriptions[j], subscriptions[j + 1]);
            }
            subscription_count--;
            printf("Unsubscribed from: %s\n", channel);
            return true;
        }
    }
    
    printf("Not subscribed to: %s\n", channel);
    return false;
}

bool websocket_authenticate(const char* access_token) {
    if (!access_token || current_status != WS_STATUS_CONNECTED) {
        return false;
    }
    
    printf("Authenticating with token: %s\n", access_token);
    
    // Simulate authentication request
    char params[1024];
    snprintf(params, sizeof(params), 
            "{\"grant_type\":\"client_credentials\",\"client_id\":\"%s\",\"client_secret\":\"%s\"}",
            "mock_client_id", "mock_client_secret");
    
    return websocket_send_request("public/auth", params, 42);
}

bool websocket_reconnect() {
    if (current_status == WS_STATUS_CONNECTED) {
        websocket_disconnect();
    }
    
    printf("Attempting reconnection to: %s\n", current_url);
    return websocket_connect(current_url, connect_cb, message_cb, error_cb, close_cb);
}

void websocket_set_auto_reconnect(bool enabled, int max_retries_val, int initial_delay_ms, int max_delay_ms_val) {
    auto_reconnect = enabled;
    max_retries = max_retries_val;
    retry_delay_ms = initial_delay_ms;
    max_delay_ms = max_delay_ms_val;
    printf("Auto-reconnect %s: max retries=%d, delay=%d-%dms\n", 
           enabled ? "enabled" : "disabled", max_retries, initial_delay_ms, max_delay_ms);
}

void websocket_set_timeout(int connect_timeout_ms, int operation_timeout_ms) {
    printf("Timeouts set: connect=%dms, operation=%dms\n", connect_timeout_ms, operation_timeout_ms);
}

void websocket_disconnect() {
    if (current_status == WS_STATUS_CONNECTED) {
        printf("Disconnecting WebSocket\n");
        if (close_cb) {
            close_cb(1000, "Normal closure");
        }
        current_status = WS_STATUS_DISCONNECTED;
    }
}

void websocket_cleanup() {
    websocket_disconnect();
    subscription_count = 0;
    printf("WebSocket client cleaned up\n");
}

WebSocketStatus websocket_get_status() {
    return current_status;
}

void websocket_process_events() {
    // Simulate processing incoming messages
    if (current_status == WS_STATUS_CONNECTED && subscription_count > 0 && rand() % 10 == 0) {
        // Randomly generate a message for an active subscription
        int sub_idx = rand() % subscription_count;
        if (message_cb) {
            WebSocketMessage msg = {
                .type = WS_MESSAGE_TEXT,
                .data = "{\n  \"type\": \"update\",\n  \"data\": {\n    \"timestamp\": 1590399378456,\n    \"price\": 25010.50\n  }\n}",
                .length = 98
            };
            strncpy(msg.channel, subscriptions[sub_idx], sizeof(msg.channel) - 1);
            message_cb(&msg);
        }
    }
}

bool websocket_is_subscribed(const char* channel) {
    if (!channel) {
        return false;
    }
    
    for (int i = 0; i < subscription_count; i++) {
        if (strcmp(subscriptions[i], channel) == 0) {
            return true;
        }
    }
    
    return false;
}

int websocket_get_subscription_count() {
    return subscription_count;
}

bool websocket_get_subscription_by_index(int index, char* channel_out, size_t channel_out_size) {
    if (index < 0 || index >= subscription_count || !channel_out || channel_out_size == 0) {
        return false;
    }
    
    strncpy(channel_out, subscriptions[index], channel_out_size - 1);
    channel_out[channel_out_size - 1] = '\0';
    return true;
}

void websocket_set_keepalive(int interval_ms) {
    printf("Keepalive interval set to %dms\n", interval_ms);
}

void websocket_build_channel_name(char* output, size_t output_size, 
                                SubscriptionType type, const char* instrument_name,
                                int interval, int depth) {
    if (!output || output_size == 0) {
        return;
    }
    
    const char* type_str = "";
    switch (type) {
        case SUBSCRIPTION_BOOK:       type_str = "book"; break;
        case SUBSCRIPTION_TRADES:     type_str = "trades"; break;
        case SUBSCRIPTION_TICKER:     type_str = "ticker"; break;
        case SUBSCRIPTION_QUOTE:      type_str = "quote"; break;
        case SUBSCRIPTION_MARK_PRICE: type_str = "mark_price"; break;
        case SUBSCRIPTION_POSITION:   type_str = "position"; break;
        case SUBSCRIPTION_ORDER:      type_str = "order"; break;
        case SUBSCRIPTION_PORTFOLIO:  type_str = "portfolio"; break;
        case SUBSCRIPTION_ANNOUNCEMENTS: type_str = "announcements"; break;
        default: type_str = "unknown"; break;
    }
    
    // Build channel name
    if (type == SUBSCRIPTION_BOOK && depth > 0) {
        snprintf(output, output_size, "%s.%s.%d", type_str, instrument_name, depth);
    } else if ((type == SUBSCRIPTION_TICKER || type == SUBSCRIPTION_MARK_PRICE) && interval > 0) {
        snprintf(output, output_size, "%s.%s.%d", type_str, instrument_name, interval);
    } else {
        snprintf(output, output_size, "%s.%s", type_str, instrument_name);
    }
}