cmake_minimum_required(VERSION 3.10)
project(GOQUANT_ASSIGNMENT C)

set(CMAKE_C_STANDARD 99)

include_directories(include)

set(SRC
    src/cJASON.C
    src/deribit_api.c
    src/orders.c
    src/websocket_client.c
    main.c
)

add_executable(main main.c)

# If your code depends on libcurl or any other libs, link them here
# find_package(CURL REQUIRED)
# target_link_libraries(main CURL::libcurl)
