<div align="center">
  <h1>Presense</h1>
  <p>An Attendance Tracker using ESP32 Network Sniffing</p>
</div>

### Overview
**Presense** is a smart attendance tracker that leverages the ESP32’s capability to sniff network probes from nearby devices. This system enables automatic tracking of attendance based on device presence without requiring manual input.

### Setup Instructions

To ensure a seamless experience, it's recommended to use **PlatformIO** for managing the ESP32 development environment. You can refer to the [PlatformIO CLI installation guide](https://platformio.org/install/integration) for detailed instructions.

1. Clone the repository:
    ```sh
    git clone https://github.com/amfoss/presense
    ```

2. Flash the ESP32 using PlatformIO:
    - Open the project in PlatformIO IDE or use the PlatformIO CLI to build and upload the firmware to the ESP32.

### Configuration

Before running the project, ensure the following fields are properly configured:

- **SSID**:
    ```cpp
    const char* ssid = "your_ssid";
    ```
  
- **PEAP Password**:
    ```cpp
    const char* password = "your_password";
    ```
  
- **PEAP Identity/Username**:
    ```cpp
    const char* username = "peap_identity";
    ```

- **GraphQL Endpoint URL**:
    ```cpp
    const char* graphql_endpoint_main = "endpoint_url";
    ```

- **Shared Secret Key**:
    ```cpp
    const char* secretKey = "secret_key";
    ```

### How to Contribute

1. Fork the repository and clone it to your local machine.
2. Set up the project by following the installation instructions above.
3. Identify an issue or feature you'd like to work on, and create an issue to track it.
4. Develop the patch or feature, ensuring it is thoroughly tested.
5. Submit a pull request, referencing the relevant issue number.

### License
This project is licensed under GNU General Public License V3. You are welcome to adapt it, make it yours. Just make sure that you credit us too.