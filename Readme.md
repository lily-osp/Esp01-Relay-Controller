# ESP8266 WiFi Relay Controller

## Overview
This project provides a versatile WiFi-controlled relay module for the ESP-01 using the ESP8266 microcontroller. The device offers remote relay control through a web interface, WebSocket, and optional Adafruit IO integration.

## Features
- WiFi-enabled relay control
- Captive portal for easy device configuration
- Persistent device state storage
- Web-based user interface
- WebSocket real-time control
- Optional Adafruit IO cloud integration
- mDNS support for easy device discovery

## Hardware Requirements
- ESP-01 module
- Relay module
- Power supply (3.3V recommended)

### Pin Configuration
- `RELAY_PIN`: GPIO0 (Controls relay)
- `LED_PIN`: GPIO1 (Status indicator)

## Setup and Configuration

### Initial Setup
1. Power on the device
2. If no WiFi credentials are configured, the device will create a captive portal
3. Connect to `ESP8266-Setup` WiFi network (password: `configme123`)
4. Open `http://192.168.4.1` in your web browser

### Configuration Options
- WiFi SSID and Password
- Custom mDNS name (e.g., `my-relay.local`)
- Optional Adafruit IO integration
  - Adafruit IO Username
  - Adafruit IO Key
  - Relay Feed Name
  - IP Feed Name

## Control Methods

### Web Interface
- Access via `http://[device-mdns-name].local`
- Toggle relay on/off directly from the web page

### WebSocket
- Real-time relay control
- Supports `ON` and `OFF` commands

### Adafruit IO (Optional)
- Cloud-based control and monitoring
- Publish and subscribe to relay state
- Periodic IP address updates

## Connectivity Features
- Automatic WiFi reconnection
- Configurable connection timeout
- Fallback to setup mode after multiple connection failures
- mDNS support for easy local network discovery

## Configuration Storage
- Device settings stored in EEPROM
- Persistent relay state memory
- Supports default configuration if no valid config found

## Debugging
- Serial output at 115200 baud
- Detailed logging for WiFi and Adafruit IO connections

## Troubleshooting
- If WiFi connection fails, device enters captive portal mode
- Reset device or reconfigure WiFi credentials if needed

## Dependencies
- ESP8266WiFi
- ESP8266WebServer
- ESP8266mDNS
- WebSocketsServer
- EEPROM
- DNSServer
- AdafruitIO_WiFi

## Recommended Power Supply
- 3.3V, minimum 500mA
- Stable power source recommended for reliable operation

## Flashing the Device
1. Use Arduino IDE or PlatformIO
2. Select "Generic ESP8266 Module"
3. Configure appropriate flash settings
4. Upload the firmware

## Security Considerations
- Change default AP password
- Use strong WiFi credentials
- Consider using Adafruit IO's security features

## License
[MIT License](LICENSE)

## Contributing
Pull requests and issues are welcome. Please follow standard GitHub contribution guidelines.

## Support
For issues or questions, please open a GitHub issue or contact the maintainer.
