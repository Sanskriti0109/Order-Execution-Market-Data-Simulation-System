# 🛠️ Order Execution & Market Data Simulation System (C)

### 👩🏻‍💻 Author: Sanskriti Saxena  
A modular, high-performance trading system prototype built in C. This project simulates the behavior of a real trading platform with mock APIs and WebSocket-based market data broadcasting.



## 🚀 What This Project Demonstrates

- ✅ Strong grasp of **C programming**
- ✅ Working with **API design & HTTP simulation**
- ✅ Structuring real-world **order management systems**
- ✅ Parsing & handling JSON using **cJSON**
- ✅ Building simulated **WebSocket clients & subscriptions**
- ✅ Clean, scalable, **modular C architecture**
- ✅ Preparedness for **low-latency trading workflows**
- ✅ Comfortable handling **complex data structures & enums**
- ✅ Focus on **performance**, **memory management**, and **structured debugging**



## 🧠 Features

### 🛒 Order Management
- Place limit/market orders
- Modify existing orders
- Cancel orders
- Track open orders

### 📊 Market & Account Data
- Simulated retrieval of orderbooks
- Track current account positions
- Print formatted summaries of market state

### 🌐 WebSocket Simulation
- Built-in mock WebSocket message stream
- Symbol subscriptions
- Real-time update simulation



## 🧱 Tech Stack

| Feature               | Technology      |
|-----------------------|-----------------|
| Core Language         | C               |
| JSON Parsing          | cJSON           |
| API Communication     | Simulated HTTP  |
| Terminal Application  | GCC + MSYS2     |
| Compilation Target    | Windows/Linux   |
| Structure             | Header/source modularity (`.h`/`.c`) |
| Design Concepts       | Enums, Structs, Callbacks, Memory Safety |

## 🔧 How to Build

### 📦 Dependencies:
- GCC Compiler (Linux/MSYS2 for Windows)
- `libcurl` (only if extended to real API)

### 🛠️ Compile

```bash
gcc main.c src/*.c -Iinclude -o trading_system -lcurl

## Output
![Image](https://github.com/user-attachments/assets/f5e1a3f9-fd8c-451c-8568-0040abb113a4)

![Image](https://github.com/user-attachments/assets/74e51f37-9b68-4836-8150-319ef9a70ead)
