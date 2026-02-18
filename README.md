# YIYAN-OS

**A High-Performance Embedded GUI System Framework for ESP32 "Cheap Yellow Display" (ESP32-2432S028R)**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: LVGL](https://img.shields.io/badge/Framework-LVGL%20v8.4-green.svg)](https://lvgl.io/)

[中文](README.zh-CN.md) | [English](README.md) | [Français](README.fr.md)

---

## Overview

YIYAN-OS is a complete embedded GUI system framework designed for the ESP32-2432S028R (Cheap Yellow Display / CYD) development board. Built on four core design principles: **Maximum Resource Utilization**, **Intelligent Optimization**, **BIOS-like Configuration**, and **Streamlined Hardware Interface**, it provides a robust, efficient, and flexible foundation.

## ✨ Key Features

### 🖥️ High-Performance Graphics Engine
- **LVGL v8.4 Integration**: Industry-leading lightweight graphics library
- **Dual-Core Parallel Processing**: Core 1 for UI rendering, Core 0 for application logic
- **Double-Buffered DMA Transfer**: Eliminates screen tearing, achieves 60 FPS
- **Partial Refresh Optimization**: Redraws only changed areas, significantly lowering CPU load

### 🔧 BIOS-like Configuration System
- **Three-Level Configuration Loading**: Code defaults → NVS persistence → SD card config files
- **Runtime Parameter Adjustment**: Modify system behavior without recompilation
- **Hardware Decoupling**: Change hardware by simply modifying configuration files

### 📱 Multi-App Management Framework
- **Application Lifecycle Management**: Active (foreground), Paused (background), Stopped (inactive)
- **Intelligent Memory Reclamation**: Background apps automatically release LVGL objects
- **Smooth App Switching**: Preloading mechanism eliminates switching delays

### ⚡ Intelligent Power Management
- **Auto Backlight Control**:
  - Manual Mode: User-adjustable brightness
  - Auto Mode: Automatic adjustment based on ambient light (requires LDR)
  - Off Mode: Backlight off
- **Idle State Management**: Enters Idle state after 30 seconds of inactivity
- **Deep Sleep**: Light Sleep after 5 minutes, wake via touch or button

### 💾 Storage System Integration
- **SPIFFS File System**: Onboard Flash storage for system resources
- **SD Card Expansion**: Large capacity for user data and media
- **LVGL File Interface**: Unified access to `F:` (Flash) and `S:` (SD card)

### 🎛️ Complete Hardware Abstraction Layer
- **Display**: ILI9341 / ST7789 driver, 320×240 resolution
- **Touchscreen**: XPT2046 resistive touch with calibration support
- **RGB LED**: Onboard tri-color indicator
- **Light Sensor**: Ambient light detection (GPIO 34)
- **BOOT Button**: Multi-function wake/mode switch

## 📊 Performance Metrics

| Metric | Value |
|--------|-------|
| Frame Rate (FPS) | ~60 |
| Memory Usage | ~105 KB (LVGL + System) |
| Flash Usage | ~800 KB |
| UI Response Latency | < 50 ms |
| App Switch Time | < 200 ms |

## 🚀 Quick Start

### Hardware Requirements
- ESP32-2432S028R (Cheap Yellow Display) development board
- microSD card (optional)
- USB data cable

### Software Environment
- PlatformIO IDE (recommended) or Arduino IDE
- ESP32 board support package v3.0+

### Build & Flash
```bash
# Clone the project
git clone <repository-url>

# Build with PlatformIO
pio run

# Flash firmware
pio run --target upload
```

## 📁 Project Structure

```
├── src/
│   ├── main.cpp           # Main entry point
│   ├── ConfigManager.*    # Configuration management
│   ├── BSP.*              # Board Support Package
│   ├── Storage.*          # Storage system
│   ├── Performance.*      # Performance monitoring
│   ├── PowerManager.*     # Power management
│   ├── AppManager.*       # Application manager
│   ├── SettingsApp.*      # Settings application
│   ├── DemoApp.*          # Demo application
│   └── lv_conf.h          # LVGL configuration
├── include/
├── lib/
└── platformio.ini
```

## � API Reference

### Application Development

```cpp
// Register new application
AppMgr.registerApp("MyApp", createMyApp, &appInfo);

// Switch to application
AppMgr.switchToApp("MyApp");

// Return to home screen
AppMgr.switchToHome();

// Set backlight brightness (0-255)
Power.setBacklight(128);

// Set backlight mode
Power.setBacklightMode(BACKLIGHT_MODE_AUTO);
```

### Application Lifecycle

```cpp
class MyApp : public BaseApp {
    bool createUI() override;    // Create UI
    void onDestroy() override;   // Cleanup
    bool onResume() override;    // Resume to foreground
    void onPause() override;     // Switch to background
    void onUpdate() override;    // Periodic update
};
```

## 📄 License

MIT License

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📧 Contact

For questions and support, please open an issue on the repository.

---

**YIYAN-OS** - *Empowering Embedded GUI Development*
