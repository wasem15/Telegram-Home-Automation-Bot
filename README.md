# ESP8266 Telegram Home Automation

An ESP8266-based smart-home prototype controlled through Telegram. The device can switch connected outputs, report light and motion sensor status, and trigger a Morse-code SOS signal through an LED.

## Features

- Telegram-based device control
- LDR light-level monitoring
- PIR motion detection
- Morse-code SOS LED signal
- Inline Telegram control buttons
- Simple authorization allow-list for Telegram chat IDs
- Wi-Fi connectivity using a local configuration header

## Hardware

| Component | Purpose |
| --- | --- |
| ESP8266 / NodeMCU | Main controller |
| Relay module | Switch connected load |
| LDR | Ambient light measurement |
| 10kΩ resistor | LDR voltage divider |
| PIR sensor | Motion detection |
| LED + 220Ω resistor | SOS indicator |
| Jumper wires / breadboard | Prototyping |

## System flow

~~~text
Telegram user
      │
      ▼
Telegram Bot API
      │
      ▼
ESP8266
 ┌────┼──────────────┐
 ▼    ▼              ▼
Relay LDR + PIR     SOS LED
~~~

## Configuration

Copy config.example.h to config.h and fill in the local credentials:

~~~cpp
#define BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
~~~

config.h is ignored by Git so local credentials are not committed.

Important: credentials that were previously present in the repository should be considered exposed. Rotate the Telegram bot token and Wi-Fi password before reusing this project.

## Arduino setup

Install the required libraries in the Arduino IDE or PlatformIO environment:

- ESP8266 board support
- UniversalTelegramBot
- WiFiClientSecure (provided by the ESP8266 framework)

Upload smart_home_bot.ino after adding your local config.h.

## Telegram commands

- /start — show the bot menu
- /switch — open device controls
- /status — show light and motion status
- /sos — trigger the Morse SOS signal

## Security notes

The prototype currently uses WiFiClientSecure::setInsecure() to simplify local ESP8266 connectivity. A production implementation should validate the Telegram server certificate.

For deployments with real devices, use an explicit allow-list of Telegram chat IDs and avoid committing device credentials.

## Project structure

~~~text
.
├── smart_home_bot.ino
├── config.example.h
├── .gitignore
├── LICENSE
└── README.md
~~~

## Project scope

This repository is an embedded/IoT prototype demonstrating communication between a microcontroller, sensors, an actuator, and a messaging platform. It is not intended to be a production home-automation security system.

## License

Apache License 2.0.
