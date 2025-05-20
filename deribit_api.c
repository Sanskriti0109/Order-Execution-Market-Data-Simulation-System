#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cJSON.h>
#include "deribit_api.h"

// Global error state
static DeribitError last_error = {DERIBIT_OK, ""};

// Set error message
static void set_error(DeribitErrorCode code, const char* message) {
    last_error.code = code;
    if (message) {
        strncpy(last_error.message, message, sizeof(last_error.message) - 1);
    } else {
        last_error.message[0] = '\0';
    }
}

// Reset error
static void reset_error() {
    last_error.code = DERIBIT_OK;
    last_error.message[0] = '\0';
}

// Internal function to initialize response data
typedef struct {
    char* data;
    size_t size;
} ResponseData;

static void init_response_data(ResponseData* response) {
    response->data = malloc(1);
    if (response->data) {
        response->data[0] = '\0';
    }
    response->size = 0;
}

// Callback function for receiving HTTP response data
static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t real_size = size * nmemb;
    ResponseData* response = (ResponseData*)userdata;
    
    char* new_data = realloc(response->data, response->size + real_size + 1);
    if (!new_data) {
        fprintf(stderr, "Failed to allocate memory for response\n");
        return 0;
    }
    
    response->data = new_data;
    memcpy(&(response->data[response->size]), ptr, real_size);
    response->size += real_size;
    response->data[response->size] = '\0';
    
    return real_size;
}

// Helper function to perform HTTP requests
// This is a placeholder for the actual curl implementation
// In a real implementation, you would use curl_easy_init(), curl_easy_setopt(), etc.
char* perform_request(const char* url, const char* post_data, const char* access_token) {
    // Simulate successful request for testing
    printf("Performing request to: %s\n", url);
    if (post_data) {
        printf("POST data: %s\n", post_data);
    }
    if (access_token) {
        printf("Using access token: %s\n", access_token);
    }
    
    // Create a dummy response for testing purposes
    // In a real implementation, this would contain the actual API response
    
    // Determine which API endpoint we're hitting to craft an appropriate dummy response
    if (strstr(url, "get_access_token") || strstr(url, "auth")) {
        // Authentication response
        char* response = malloc(256);
        sprintf(response, "{\"result\":{\"access_token\":\"dummy_token\",\"expires_in\":3600},\"usIn\":1234567890,\"usOut\":1234567891,\"usDiff\":1,\"testnet\":true}");
        return response;
    } 
    else if (strstr(url, "get_order_book")) {
        // Orderbook response
        char* response = malloc(1024);
        sprintf(response, 
            "{\"result\":{"
            "\"bids\":[[25000.0,0.5],[24950.0,1.2],[24900.0,0.8]],"
            "\"asks\":[[25050.0,0.3],[25100.0,0.9],[25150.0,1.5]],"
            "\"timestamp\":1621234567890"
            "},\"usIn\":1234567890,\"usOut\":1234567891,\"usDiff\":1,\"testnet\":true}");
        return response;
    } 
    else if (strstr(url, "get_positions")) {
        // Positions response
        char* response = malloc(1024);
        sprintf(response, 
            "{\"result\":["
            "{\"instrument_name\":\"BTC-PERPETUAL\",\"size\":0.5,\"average_price\":25000.0,\"mark_price\":25050.0,\"floating_profit_loss\":0.00125,\"realized_profit_loss\":0.0035,\"last_update_timestamp\":1621234567890}"
            "],\"usIn\":1234567890,\"usOut\":1234567891,\"usDiff\":1,\"testnet\":true}");
        return response;
    } 
    else if (strstr(url, "get_open_orders")) {
        // Open orders response
        char* response = malloc(1024);
        sprintf(response, 
            "{\"result\":["
            "{\"order_id\":\"ETH-12345\",\"instrument_name\":\"BTC-PERPETUAL\",\"price\":25000.0,\"amount\":0.1,\"direction\":\"buy\",\"order_type\":\"limit\",\"order_state\":\"open\",\"creation_timestamp\":1621234567890,\"last_update_timestamp\":1621234567890}"
            "],\"usIn\":1234567890,\"usOut\":1234567891,\"usDiff\":1,\"testnet\":true}");
        return response;
    }
    else if (strstr(url, "get_instruments")) {
        // Instruments response
        char* response = malloc(1024);
        sprintf(response, 
            "{\"result\":["
            "{\"instrument_name\":\"BTC-PERPETUAL\",\"kind\":\"future\",\"base_currency\":\"BTC\",\"quote_currency\":\"USD\",\"tick_size\":0.5,\"min_trade_amount\":0.001},"
            "{\"instrument_name\":\"ETH-PERPETUAL\",\"kind\":\"future\",\"base_currency\":\"ETH\",\"quote_currency\":\"USD\",\"tick_size\":0.05,\"min_trade_amount\":0.01}"
            "],\"usIn\":1234567890,\"usOut\":1234567891,\"usDiff\":1,\"testnet\":true}");
        return response;
    }
    else if (strstr(url, "get_account_summary")) {
        // Account summary response
        char* response = malloc(1024);
        sprintf(response, 
            "{\"result\":{"
            "\"equity\":1.0,\"available_funds\":0.9,\"balance\":1.0,\"initial_margin\":0.1,\"maintenance_margin\":0.05,"
            "\"margin_balance\":1.0,\"session_upl\":0.01,\"session_rpl\":0.005,\"session_funding\":0.001"
            "},\"usIn\":1234567890,\"usOut\":1234567891,\"usDiff\":1,\"testnet\":true}");
        return response;
    }
    else if (strstr(url, "get_trades")) {
        // Trades response
        char* response = malloc(1024);
        sprintf(response, 
            "{\"result\":["
            "{\"trade_id\":\"12345\",\"instrument_name\":\"BTC-PERPETUAL\",\"price\":25000.0,\"amount\":0.1,\"direction\":\"buy\",\"timestamp\":1621234567890},"
            "{\"trade_id\":\"12346\",\"instrument_name\":\"BTC-PERPETUAL\",\"price\":25005.0,\"amount\":0.2,\"direction\":\"sell\",\"timestamp\":1621234567891}"
            "],\"usIn\":1234567890,\"usOut\":1234567891,\"usDiff\":1,\"testnet\":true}");
        return response;
    }
    else if (strstr(url, "get_ticker")) {
        // Ticker response
        char* response = malloc(1024);
        sprintf(response, 
            "{\"result\":{"
            "\"instrument_name\":\"BTC-PERPETUAL\",\"last_price\":25000.0,\"best_bid_price\":24995.0,\"best_ask_price\":25005.0,"
            "\"best_bid_amount\":0.5,\"best_ask_amount\":0.3,\"mark_price\":25001.0,\"index_price\":25002.0,\"volume\":100.5,"
            "\"open_interest\":1000.0,\"timestamp\":1621234567890"
            "},\"usIn\":1234567890,\"usOut\":1234567891,\"usDiff\":1,\"testnet\":true}");
        return response;
    }
    else {
        // Generic response for other endpoints
        char* response = malloc(256);
        sprintf(response, "{\"result\":{\"success\":true},\"usIn\":1234567890,\"usOut\":1234567891,\"usDiff\":1,\"testnet\":true}");
        return response;
    }
}

// Parse API error from response
static int parse_api_error(const char *json_response) {
    cJSON *json = cJSON_Parse(json_response);
    if (!json) {
        set_error(DERIBIT_ERROR_INTERNAL, "Failed to parse JSON response");
        return 0;
    }
    
    cJSON *error = cJSON_GetObjectItemCaseSensitive(json, "error");
    if (cJSON_IsObject(error)) {
        cJSON *message = cJSON_GetObjectItemCaseSensitive(error, "message");
        cJSON *code = cJSON_GetObjectItemCaseSensitive(error, "code");
        
        if (cJSON_IsString(message) && cJSON_IsNumber(code)) {
            set_error(DERIBIT_ERROR_PARAMS, message->valuestring);
        } else {
            set_error(DERIBIT_ERROR_INTERNAL, "Unknown API error");
        }
        
        cJSON_Delete(json);
        return 0;
    }
    
    cJSON_Delete(json);
    return 1;
}

// Get the last error
DeribitError get_last_error() {
    return last_error;
}

// Authentication
bool get_access_token(const char* client_id, const char* client_secret, char* access_token) {
    reset_error();
    
    if (!client_id || !client_secret || !access_token) {
        set_error(DERIBIT_ERROR_PARAMS, "Invalid parameters for authentication");
        return false;
    }

    char url[256] = {0};
    char post_data[512] = {0};
    
    snprintf(url, sizeof(url), "https://www.deribit.com/api/v2/public/auth");
    snprintf(post_data, sizeof(post_data), 
        "{\"grant_type\":\"client_credentials\",\"client_id\":\"%s\",\"client_secret\":\"%s\"}",
        client_id, client_secret);
    
    char* response = perform_request(url, post_data, NULL);
    if (!response) {
        set_error(DERIBIT_ERROR_NETWORK, "Failed to connect to Deribit API");
        return false;
    }
    
    if (!parse_api_error(response)) {
        free(response);
        return false;
    }
    
    cJSON* json = cJSON_Parse(response);
    if (!json) {
        set_error(DERIBIT_ERROR_INTERNAL, "Failed to parse JSON response");
        free(response);
        return false;
    }
    
    cJSON* result = cJSON_GetObjectItemCaseSensitive(json, "result");
    if (!cJSON_IsObject(result)) {
        set_error(DERIBIT_ERROR_INTERNAL, "Invalid API response format");
        cJSON_Delete(json);
        free(response);
        return false;
    }
    
    cJSON* token = cJSON_GetObjectItemCaseSensitive(result, "access_token");
    if (!cJSON_IsString(token) || !token->valuestring) {
        set_error(DERIBIT_ERROR_INTERNAL, "No access token in response");
        cJSON_Delete(json);
        free(response);
        return false;
    }
    #define TOKEN_SIZE 128
    strncpy(access_token, token->valuestring, TOKEN_SIZE - 1);
    access_token[TOKEN_SIZE - 1] = '\0';
    
    cJSON_Delete(json);
    free(response);
    return true;
}

// Get instruments
void get_instruments(const char* currency, const char* kind, const char* access_token) {
    reset_error();
    
    if (!currency || !kind) {
        set_error(DERIBIT_ERROR_PARAMS, "Invalid parameters for get_instruments");
        return;
    }
    
    char url[256] = {0};
    char post_data[512] = {0};
    
    snprintf(url, sizeof(url), "https://www.deribit.com/api/v2/public/get_instruments");
    snprintf(post_data, sizeof(post_data), 
        "{\"currency\":\"%s\",\"kind\":\"%s\"}",
        currency, kind);
    
    char* response = perform_request(url, post_data, access_token);
    if (!response) {
        set_error(DERIBIT_ERROR_NETWORK, "Failed to connect to Deribit API");
        return;
    }
    
    if (!parse_api_error(response)) {
        free(response);
        return;
    }
    
    // Print the response for debugging
    printf("Instruments response: %s\n", response);
    
    free(response);
}

// Get ticker
void get_ticker(const char* instrument_name, const char* access_token) {
    reset_error();
    
    if (!instrument_name) {
        set_error(DERIBIT_ERROR_PARAMS, "Invalid parameters for get_ticker");
        return;
    }
    
    char url[256] = {0};
    char post_data[512] = {0};
    
    snprintf(url, sizeof(url), "https://www.deribit.com/api/v2/public/get_ticker");
    snprintf(post_data, sizeof(post_data), 
        "{\"instrument_name\":\"%s\"}",
        instrument_name);
    
    char* response = perform_request(url, post_data, access_token);
    if (!response) {
        set_error(DERIBIT_ERROR_NETWORK, "Failed to connect to Deribit API");
        return;
    }
    
    if (!parse_api_error(response)) {
        free(response);
        return;
    }
    
    // Print the response for debugging
    printf("Ticker response: %s\n", response);
    
    free(response);
}

// Get trades
void get_trades(const char* instrument_name, int count, const char* access_token) {
    reset_error();
    
    if (!instrument_name || count <= 0) {
        set_error(DERIBIT_ERROR_PARAMS, "Invalid parameters for get_trades");
        return;
    }
    
    char url[256] = {0};
    char post_data[512] = {0};
    
    snprintf(url, sizeof(url), "https://www.deribit.com/api/v2/public/get_last_trades_by_instrument");
    snprintf(post_data, sizeof(post_data), 
        "{\"instrument_name\":\"%s\",\"count\":%d}",
        instrument_name, count);
    
    char* response = perform_request(url, post_data, access_token);
    if (!response) {
        set_error(DERIBIT_ERROR_NETWORK, "Failed to connect to Deribit API");
        return;
    }
    
    if (!parse_api_error(response)) {
        free(response);
        return;
    }
    
    // Print the response for debugging
    printf("Trades response: %s\n", response);
    
    free(response);
}

// Get account summary
void get_account_summary(const char* currency, const char* access_token) {
    reset_error();
    
    if (!currency || !access_token) {
        set_error(DERIBIT_ERROR_PARAMS, "Invalid parameters for get_account_summary");
        return;
    }
    
    char url[256] = {0};
    char post_data[512] = {0};
    
    snprintf(url, sizeof(url), "https://www.deribit.com/api/v2/private/get_account_summary");
    snprintf(post_data, sizeof(post_data), 
        "{\"currency\":\"%s\"}",
        currency);
    
    char* response = perform_request(url, post_data, access_token);
    if (!response) {
        set_error(DERIBIT_ERROR_NETWORK, "Failed to connect to Deribit API");
        return;
    }
    
    if (!parse_api_error(response)) {
        free(response);
        return;
    }
    
    // Print the response for debugging
    printf("Account summary response: %s\n", response);
    
    free(response);
}

// Get orderbook
bool get_orderbook(const char* instrument_name, int depth, const char* access_token, OrderBook* orderbook) {
    reset_error();
    
    if (!instrument_name || depth <= 0 || !orderbook) {
        set_error(DERIBIT_ERROR_PARAMS, "Invalid parameters for get_orderbook");
        return false;
    }
    
    char url[256] = {0};
    char post_data[512] = {0};
    
    snprintf(url, sizeof(url), "https://www.deribit.com/api/v2/public/get_order_book");
    snprintf(post_data, sizeof(post_data), 
        "{\"instrument_name\":\"%s\",\"depth\":%d}",
        instrument_name, depth);
    
    char* response = perform_request(url, post_data, access_token);
    if (!response) {
        set_error(DERIBIT_ERROR_NETWORK, "Failed to connect to Deribit API");
        return false;
    }
    
    if (!parse_api_error(response)) {
        free(response);
        return false;
    }
    
    cJSON* json = cJSON_Parse(response);
    if (!json) {
        set_error(DERIBIT_ERROR_INTERNAL, "Failed to parse JSON response");
        free(response);
        return false;
    }
    
    cJSON* result = cJSON_GetObjectItemCaseSensitive(json, "result");
    if (!cJSON_IsObject(result)) {
        set_error(DERIBIT_ERROR_INTERNAL, "Invalid API response format");
        cJSON_Delete(json);
        free(response);
        return false;
    }
    
    // Process bids
    cJSON* bids = cJSON_GetObjectItemCaseSensitive(result, "bids");
    if (cJSON_IsArray(bids)) {
        int bids_count = cJSON_GetArraySize(bids);
        orderbook->bids_count = bids_count;
        orderbook->bids = (OrderBookEntry*)malloc(bids_count * sizeof(OrderBookEntry));
        
        for (int i = 0; i < bids_count; i++) {
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
    } else {
        orderbook->bids_count = 0;
        orderbook->bids = NULL;
    }
    
    // Process asks
    cJSON* asks = cJSON_GetObjectItemCaseSensitive(result, "asks");
    if (cJSON_IsArray(asks)) {
        int asks_count = cJSON_GetArraySize(asks);
        orderbook->asks_count = asks_count;
        orderbook->asks = (OrderBookEntry*)malloc(asks_count * sizeof(OrderBookEntry));
        
        for (int i = 0; i < asks_count; i++) {
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
    } else {
        orderbook->asks_count = 0;
        orderbook->asks = NULL;
    }
    
    // Get timestamp
    cJSON* timestamp = cJSON_GetObjectItemCaseSensitive(result, "timestamp");
    if (cJSON_IsNumber(timestamp)) {
        time_t ts = (time_t)(timestamp->valuedouble / 1000);
        struct tm* timeinfo = gmtime(&ts);
        strftime(orderbook->timestamp, sizeof(orderbook->timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    } else {
        orderbook->timestamp[0] = '\0';
    }
    
    cJSON_Delete(json);
    free(response);
    return true;
}

// Get positions
bool get_positions(const char* currency, const char* kind, const char* access_token, Position** positions, int* positions_count) {
    reset_error();
    
    if (!currency || !kind || !access_token || !positions || !positions_count) {
        set_error(DERIBIT_ERROR_PARAMS, "Invalid parameters for get_positions");
        return false;
    }
    
    char url[256] = {0};
    char post_data[512] = {0};
    
    snprintf(url, sizeof(url), "https://www.deribit.com/api/v2/private/get_positions");
    snprintf(post_data, sizeof(post_data), 
        "{\"currency\":\"%s\",\"kind\":\"%s\"}",
        currency, kind);
    
    char* response = perform_request(url, post_data, access_token);
    if (!response) {
        set_error(DERIBIT_ERROR_NETWORK, "Failed to connect to Deribit API");
        return false;
    }
    
    if (!parse_api_error(response)) {
        free(response);
        return false;
    }
    
    cJSON* json = cJSON_Parse(response);
    if (!json) {
        set_error(DERIBIT_ERROR_INTERNAL, "Failed to parse JSON response");
        free(response);
        return false;
    }
    
    cJSON* result = cJSON_GetObjectItemCaseSensitive(json, "result");
    if (!cJSON_IsArray(result)) {
        set_error(DERIBIT_ERROR_INTERNAL, "Invalid API response format");
        cJSON_Delete(json);
        free(response);
        return false;
    }
    
    int count = cJSON_GetArraySize(result);
    *positions_count = count;
    
    if (count > 0) {
        *positions = (Position*)malloc(count * sizeof(Position));
        
        for (int i = 0; i < count; i++) {
            cJSON* pos = cJSON_GetArrayItem(result, i);
            
            cJSON* instrument_name = cJSON_GetObjectItemCaseSensitive(pos, "instrument_name");
            cJSON* size = cJSON_GetObjectItemCaseSensitive(pos, "size");
            cJSON* average_price = cJSON_GetObjectItemCaseSensitive(pos, "average_price");
            cJSON* mark_price = cJSON_GetObjectItemCaseSensitive(pos, "mark_price");
            cJSON* floating_pl = cJSON_GetObjectItemCaseSensitive(pos, "floating_profit_loss");
            cJSON* realized_pl = cJSON_GetObjectItemCaseSensitive(pos, "realized_profit_loss");
            cJSON* last_update_timestamp = cJSON_GetObjectItemCaseSensitive(pos, "last_update_timestamp");
            
            Position* p = &(*positions)[i];
            
            if (cJSON_IsString(instrument_name) && instrument_name->valuestring) {
                strncpy(p->instrument_name, instrument_name->valuestring, sizeof(p->instrument_name) - 1);
            } else {
                p->instrument_name[0] = '\0';
            }
            
            p->size = cJSON_IsNumber(size) ? size->valuedouble : 0.0;
            p->entry_price = cJSON_IsNumber(average_price) ? average_price->valuedouble : 0.0;
            p->mark_price = cJSON_IsNumber(mark_price) ? mark_price->valuedouble : 0.0;
            p->unrealized_pnl = cJSON_IsNumber(floating_pl) ? floating_pl->valuedouble : 0.0;
            p->realized_pnl = cJSON_IsNumber(realized_pl) ? realized_pl->valuedouble : 0.0;
            
            if (cJSON_IsNumber(last_update_timestamp)) {
    time_t ts = (time_t)(last_update_timestamp->valuedouble / 1000);
    struct tm* timeinfo = gmtime(&ts);
    if (timeinfo != NULL) {
        // Manually build the timestamp string without using % formatting
        char year[5], month[3], day[3], hour[3], minute[3], second[3];

        // Convert to strings with leading zeroes manually
        snprintf(year, sizeof(year), "%d", timeinfo->tm_year + 1900);
        snprintf(month, sizeof(month), "%02d", timeinfo->tm_mon + 1);
        snprintf(day, sizeof(day), "%02d", timeinfo->tm_mday);
        snprintf(hour, sizeof(hour), "%02d", timeinfo->tm_hour);
        snprintf(minute, sizeof(minute), "%02d", timeinfo->tm_min);
        snprintf(second, sizeof(second), "%02d", timeinfo->tm_sec);

        // Combine manually
        snprintf(p->timestamp, sizeof(p->timestamp),
                 "%s-%s-%s %s:%s:%s",
                 year, month, day, hour, minute, second);
    } else {
        p->timestamp[0] = '\0';
    }
} else {
    p->timestamp[0] = '\0';
}

        }
    } else {
        *positions = NULL;
    }
    
    cJSON_Delete(json);
    free(response);
    return true;
}