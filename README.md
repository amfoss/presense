<div align="center">
  <h1>Presense</h1>
  <p>An Attendance Tracker using ESP32 Network Sniffing</p>
</div>

### Overview
**Presense** is a smart attendance tracker that leverages the ESP32’s capability to sniff network probes from nearby devices. This system enables automatic tracking of attendance based on device presence without requiring manual input.

### Setup Instructions

To ensure a seamless experience, it's recommended to use **PlatformIO** for managing the ESP32 development environment. Install PlatformIO Core via pip:

```sh
pip install platformio
```

1. Clone the repository:
    ```sh
    git clone https://github.com/amfoss/presense
    ```

2. Configure secrets:
    ```sh
    cp src/include/secrets.h.sample src/include/secrets.h
    ```
    Then edit `src/include/secrets.h` with your configuration values.

3. Build and flash the ESP32 using PlatformIO:
    ```sh
    pio run -t upload
    ```

### Configuration

Before running the project, copy `src/include/secrets.h.sample` to `src/include/secrets.h` and configure the following variables:

- **SSID**: WiFi network name to connect to
    ```cpp
    #define SSID "wifi_network"
    ```
  
- **USERNAME**: PEAP/Enterprise WiFi username (leave empty for WPA2-PSK networks)
    ```cpp
    #define USERNAME "username"
    ```
  
- **PASSWORD**: WiFi password (or PEAP password for enterprise networks)
    ```cpp
    #define PASSWORD "password"
    ```

- **SECRET_KEY**: Shared secret key for HMAC signature generation (used to authenticate attendance records)
    ```cpp
    #define SECRET_KEY "secret_key"
    ```

- **GRAPHQL_ENDPOINT**: Backend GraphQL API endpoint URL for fetching member data and submitting attendance
    ```cpp
    #define GRAPHQL_ENDPOINT "api_url"
    ```

- **TIME_API_KEY**: API key for TimezoneDB service (used to fetch accurate timestamp)
    ```cpp
    #define TIME_API_KEY "timezonedb_api_key"
    ```

- **MAC_ADDRESS**: Custom MAC address to spoof on the ESP32 (hex format)
    ```cpp
    #define MAC_ADDRESS {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
    ```

- **IS_ENTERPRISE**: Set to `true` for WPA2-Enterprise (PEAP) networks, `false` for WPA2-PSK
    ```cpp
    #define IS_ENTERPRISE true
    ```

### How to Contribute

1. Fork the repository and clone it to your local machine.
2. Set up the project by following the installation instructions above.
3. Identify an issue or feature you'd like to work on, and create an issue to track it.
4. Develop the patch or feature, ensuring it is thoroughly tested.
5. Submit a pull request, referencing the relevant issue number.

### License
This project is licensed under GNU General Public License V3. You are welcome to adapt it, make it yours. Just make sure that you credit us too.