#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// WebSocket connection status
typedef enum {
    WS_STATUS_DISCONNECTED,
    WS_STATUS_CONNECTING,
    WS_STATUS_CONNECTED,
    WS_STATUS_ERROR
} WebSocketStatus;

// WebSocket message types
typedef enum {
    WS_MESSAGE_TEXT,
    WS_MESSAGE_BINARY,
    WS_MESSAGE_PING,
    WS_MESSAGE_PONG,
    WS_MESSAGE_CLOSE
} WebSocketMessageType;

// WebSocket message structure
typedef struct {
    WebSocketMessageType type;
    char* data;
    size_t length;
    char channel[128];  // Added channel field to track message source
} WebSocketMessage;

// Subscription types for Deribit
typedef enum {
    SUBSCRIPTION_BOOK,         // Order book
    SUBSCRIPTION_TRADES,       // Trades
    SUBSCRIPTION_TICKER,       // Ticker
    SUBSCRIPTION_QUOTE,        // Quote
    SUBSCRIPTION_MARK_PRICE,   // Mark price
    SUBSCRIPTION_POSITION,     // User position
    SUBSCRIPTION_ORDER,        // User order
    SUBSCRIPTION_PORTFOLIO,    // User portfolio
    SUBSCRIPTION_ANNOUNCEMENTS // Announcements
} SubscriptionType;

// Subscription request structure
typedef struct {
    SubscriptionType type;
    char instrument_name[32];
    int depth;               // For order book
    int interval;            // For tickers, mark prices
    bool is_snapshot;        // Whether to request initial snapshot
} SubscriptionRequest;

// Callback function types
typedef void (*WebSocketConnectCallback)(bool success, const char* message);
typedef void (*WebSocketMessageCallback)(const WebSocketMessage* message);
typedef void (*WebSocketErrorCallback)(const char* error);
typedef void (*WebSocketCloseCallback)(int code, const char* reason);
typedef void (*WebSocketSubscriptionCallback)(const char* channel, bool success);

// Initialize WebSocket client
bool websocket_init();

// Connect to WebSocket server
bool websocket_connect(const char* url, 
                      WebSocketConnectCallback connect_callback,
                      WebSocketMessageCallback message_callback,
                      WebSocketErrorCallback error_callback,
                      WebSocketCloseCallback close_callback);

// Send message
bool websocket_send(const char* message);

// Send binary message
bool websocket_send_binary(const unsigned char* data, size_t length);

// Send JSON-RPC request
bool websocket_send_request(const char* method, const char* params, int request_id);

// Subscribe to channel with parameters
bool websocket_subscribe_with_params(const SubscriptionRequest* request, 
                                    WebSocketSubscriptionCallback callback);

// Subscribe to channel (simplified)
bool websocket_subscribe(const char* channel, WebSocketSubscriptionCallback callback);

// Unsubscribe from channel
bool websocket_unsubscribe(const char* channel);

// Authenticate with access token
bool websocket_authenticate(const char* access_token);

// Reconnect with exponential backoff
bool websocket_reconnect();

// Set auto-reconnect options
void websocket_set_auto_reconnect(bool enabled, int max_retries, int initial_delay_ms, int max_delay_ms);

// Set connection timeout
void websocket_set_timeout(int connect_timeout_ms, int operation_timeout_ms);

// Disconnect from WebSocket server
void websocket_disconnect();

// Cleanup WebSocket client
void websocket_cleanup();

// Get current WebSocket status
WebSocketStatus websocket_get_status();

// Process WebSocket events (call in main loop)
void websocket_process_events();

// Check if currently subscribed to a channel
bool websocket_is_subscribed(const char* channel);

// Get active subscription count
int websocket_get_subscription_count();

// Get subscription by index
bool websocket_get_subscription_by_index(int index, char* channel_out, size_t channel_out_size);

// Set keep-alive interval
void websocket_set_keepalive(int interval_ms);

// Generate specific Deribit subscription channel name
void websocket_build_channel_name(char* output, size_t output_size, 
                                 SubscriptionType type, const char* instrument_name,
                                 int interval, int depth);

#ifdef __cplusplus
}
#endif

#endif // WEBSOCKET_CLIENT_H
