#ifndef DERIBIT_API_H
#define DERIBIT_API_H
#include "order.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
char* perform_request(const char* url, const char* post_data, const char* access_token);

// Authentication functions
bool get_access_token(const char* client_id, const char* client_secret, char* access_token);

// Market data functions
void get_instruments(const char* currency, const char* kind, const char* access_token);

void get_ticker(const char* instrument_name, const char* access_token);
void get_trades(const char* instrument_name, int count, const char* access_token);

// Account functions
void get_account_summary(const char* currency, const char* access_token);
bool get_orderbook(const char* instrument_name, int depth, const char* access_token, OrderBook* orderbook);
bool get_positions(const char* currency, const char* kind, const char* access_token, Position** positions, int* positions_count);


// Error handling
typedef enum {
    DERIBIT_OK = 0,
    DERIBIT_ERROR_NETWORK,
    DERIBIT_ERROR_AUTH,
    DERIBIT_ERROR_PARAMS,
    DERIBIT_ERROR_RATE_LIMIT,
    DERIBIT_ERROR_INTERNAL
} DeribitErrorCode;

typedef struct {
    DeribitErrorCode code;
    char message[256];
} DeribitError;

// Get last error
DeribitError get_last_error();

#ifdef __cplusplus
}
#endif

#endif // DERIBIT_API_H
