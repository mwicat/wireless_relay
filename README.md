# Wireless relay

A program for ESP32 microcontroller that makes it a wireless relay controller.

Uses PlatformIO IDE and arduino framework for ESP32.

Features:

1. Connects to WiFi and listens on specified port
2. Serves an API endpoint that allows to switch on and switch off relay connected to MCU digital output
3. Serves a webpage that allows to switch on and switch off relay connected to MCU digital output
4. Allows to specify and save WiFi and other credentials through serial port communication

## API endpoints

- `/api/v1/switch` - send a `POST` request with content-type `application/x-www-form-urlencoded` with argument `enabled = 0|1`

Example 1 - turn relay on:

```shell
curl -X POST "http://[CONTROLLER IP]/api/v1/switch"
   -H "Content-Type: application/x-www-form-urlencoded" 
   -d "enabled=1" 
```

## Webpages

- `/` - main page with simple interface for interacting with relay setting

## Compilation

```shell
pio run
```
