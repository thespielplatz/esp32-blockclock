# ESP32 NeoPixel Matrix Controller

This project is a firmware for controlling a custom-built NeoPixel (WS2812) LED matrix using an ESP32.  
It includes a web-based user interface for configuration and handles Wi-Fi auto-connection, LED layout control, and current limiting.

> ⚠️ **Note:** The physical LED matrix hardware (50x5 NeoPixels) is **not included in this repository**.

---

## 💡 Features

- 🔧 **Web-based Configuration UI**  
  Easily configure matrix parameters and animations via a local web interface.

- 📶 **Auto Wi-Fi Connect**  
  Automatically reconnects to known Wi-Fi or offers fallback hotspot mode for first-time setup.

- 🔌 **Custom Matrix Support (50×5 LEDs)**  
  Designed for a 250-LED matrix (50 columns × 5 rows), but can be adjusted in settings.

- 🔢 **Pin Configuration**  
  Define data pin, power pin, or other GPIO-related settings via config.

- ⚡ **Max Current Limiting**  
  Set the maximum current draw to protect your power supply and hardware.

---

## 🛠️ Hardware Requirements

- **ESP32 Dev Board** (e.g. ESP32-WROOM-32)
- **Custom-built NeoPixel Matrix**  
  50 columns × 5 rows, using WS2812 (or compatible) LEDs
- **5V Power Supply** with enough current capacity (≥ 15A recommended for full brightness)
- **Level Shifter** (optional but recommended for 5V pixels)

---

## 🚀 Getting Started

1. **Clone this repo** and open it in [PlatformIO](https://platformio.org/).
2. Connect your ESP32 board.
3. Configure Wi-Fi and pin settings (or use the Web UI after first boot).
4. Upload the firmware:
   ```bash
   pio run -t upload


## Docs

- ESP Web Tools: https://esphome.github.io/esp-web-tools/
- Improv Serial: https://www.improv-wifi.com/serial/
- Imporv C++ SDK: https://github.com/improv-wifi/sdk-cpp/blob/main/README.md
- Improv Webserver Demo: https://jnthas.github.io/improv-wifi-demo/
- Improv Webserver Demo Source: https://github.com/jnthas/improv-wifi-demo/blob/main/src/esp32-wifiimprov/esp32-wifiimprov.ino
