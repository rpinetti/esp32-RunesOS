# ESP32-RunesOS Architecture Analysis

## Executive Summary

**RunesOS** is a minimalist embedded operating system for the ESP32-S3 based on a cyberdeck housed in a Barkley's tin. It features a 480x640 RGB LCD display (ST7701 driver), capacitive touch (GT911), and BLE keyboard connectivity. The architecture follows a clean layered design separating hardware abstraction from application logic.

---

## System Overview

### Hardware Stack
| Component | Specification | Purpose |
|-----------|---------------|---------|
| **MCU** | ESP32-S3 (8MB PSRAM, 16MB flash) | Main processor |
| **Display** | 480x640 RGB LCD (ST7701) | Visual output |
| **Touch** | GT911 capacitive controller | Input |
| **RTC** | PCF85063 | Time/date persistence |
| **IMU** | QMI8658 (6-axis) | Motion detection |
| **Storage** | SD card | File storage |
| **Audio** | Buzzer + Speaker | Sound feedback |
| **Battery** | LiPo with charging circuit | Power management |
| **Connectivity** | WiFi + BLE (keyboard) | Radio interfaces |

### Target Device
- **Form Factor**: Portable mini-device in a candy tin
- **Display**: Smartphone-like 480x640 vertical orientation
- **Operating System**: Real-time kernel (FreeRTOS)
- **UI Framework**: LVGL (Light and Versatile Graphics Library)
- **Build System**: PlatformIO (ESP32-S3 toolchain)

---

## Architectural Layers

```
┌─────────────────────────────────────────────────────────┐
│                   UI Layer (runesos-ui)                  │
│  ┌──────────────────────────────────────────────────┐   │
│  │ Screens: Lockscreen, Homescreen, Statusbar      │   │
│  │ Apps: Oraculo, Relogio, Notas, Terminal, etc.  │   │
│  │ Theme: Nordico (theme_nordico.c/h)             │   │
│  │ Framework: LVGL 9.x                             │   │
│  └──────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────┤
│              Firmware Layer (firmware/)                  │
│  ┌──────────────────────────────────────────────────┐   │
│  │ Services: Battery, Radio, Time                   │   │
│  │ Runtime: FreeRTOS tasks, timers, ISRs           │   │
│  │ Coordination: Task scheduling, event handling    │   │
│  └──────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────┤
│          Hardware Abstraction Layer (firmware/hal/)      │
│  ┌──────────────────────────────────────────────────┐   │
│  │ Abstraction interfaces for:                       │   │
│  │  • RTC (Time)         • Audio (Buzzer/Speaker)  │   │
│  │  • Battery            • Backlight control        │   │
│  │  • Radio (WiFi/BLE)   • Power management        │   │
│  │  • IMU (6-axis)       • SD Card I/O             │   │
│  └──────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────┤
│               Platform Layer (lv_port_pc_vscode/)        │
│  ┌──────────────────────────────────────────────────┐   │
│  │ Board Support Package (BSP)                      │   │
│  │ Driver implementations for ESP32-S3 peripherals │   │
│  │ PC simulator fallbacks for development           │   │
│  │ FreeRTOS + LVGL integration                      │   │
│  └──────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────┤
│      Embedded Runtime (FreeRTOS + LVGL Libraries)       │
│  ├─ Real-time kernel (task scheduling, IPC)            │
│  ├─ Graphics library (display rendering)                │
│  ├─ Peripheral drivers (ST7701, GT911, etc.)           │
│  └─ I/O abstraction (UART, I2C, SPI, etc.)             │
└─────────────────────────────────────────────────────────┘
```

---

## Module Structure

### 1. HAL Layer (`firmware/hal/`)

**Purpose**: Decouple hardware-specific code from application logic.

**Key Headers**:
- `hal_runesos.h` - Master HAL interface with function pointers
- `hal_battery.h` - Battery monitoring (percent, voltage, charging status)
- `hal_radio.h` - WiFi/BLE state (signal strength, connection status)
- `hal_time.h` - RTC access (get/set time with struct containing year, month, day, etc.)

**Design Pattern**: Function pointer table
```c
typedef struct {
    void (*get_time)(runesos_hal_time_t *t);
    void (*get_battery)(runesos_hal_battery_t *b);
    void (*get_radio)(runesos_hal_radio_t *r);
    void (*get_imu)(runesos_hal_imu_t *imu);
    void (*get_sd)(runesos_hal_sd_t *sd);
    void (*buzzer_on)(uint16_t freq_hz, uint16_t duration_ms);
    void (*set_backlight)(uint8_t percent);
    void (*enter_sleep)(void);
    // ... more operations
} runesos_hal_t;
```

**Benefits**:
- Single interface for both real hardware and PC simulator
- Easy to mock for testing
- Clean separation of concerns

---

### 2. Services Layer (`firmware/services/`)

**Purpose**: Implement business logic and state management.

**Available Services**:

#### Battery Service (`battery_service.c/h`)
- Monitors battery voltage and charging state
- Exposes `runesos_battery_get(state)` API
- Data type: `runesos_battery_state_t`

#### Radio Service (`radio_service.c/h`)
- Manages WiFi and BLE connectivity
- Tracks WiFi signal strength (RSSI → bars)
- Manages BLE keyboard pairing
- Exposes `runesos_radio_get(state)` API

#### Time Service (`time_service.c/h`)
- Wraps RTC access
- Provides time synchronization utilities
- Exposes `runesos_time_get(time)` API

**Design Pattern**: Service abstraction over HAL
- Each service translates HAL data into business logic
- UI queries services, not HAL directly
- Services can aggregate multiple HAL calls

---

### 3. Firmware Layer (`firmware/`)

**Main Entry**: `runesos_fw.c/h`
- `runesos_fw_init()` - Initialize all subsystems

**Responsibilities**:
- Orchestrate service startup
- Manage FreeRTOS task creation
- Handle interrupt service routines (ISRs)
- Coordinate communication between layers

**Architecture**: Task-driven with event queues
- Each service typically runs in its own FreeRTOS task
- Inter-task communication via queues or shared state with mutexes
- Timer interrupts drive periodic updates (battery polling, clock tick, etc.)

---

### 4. UI Layer (`runesos-ui/`)

**Core Components**:

#### Main Module (`runesos.c/h`)
- Entry point: `runesos_init()` 
- Initializes UI subsystem
- Launches launcher

#### Session Management (`runesos_session.c/h`)
- Manages app lifecycle
- Transitions between lockscreen → homescreen → apps
- Handles unlock logic and app launch callbacks

#### Screen Components
- **Lockscreen** (`runesos_lockscreen.c/h`): Time display, unlock animation, status indicators
- **Homescreen** (`runesos_homescreen.c/h`): App grid/launcher view
- **Statusbar** (`runesos_statusbar.c/h`): Top bar showing battery, signal, time
- **Launcher** (`launcher/launcher.c/h`): App selection interface

#### App Registry (`runesos_app_registry.c`)
**Available Apps** (plugin architecture):
- `app_oraculo` - Rune divination / fortune teller
- `app_relogio` - Clock display with time settings
- `app_notas` - Note-taking application
- `app_terminal` - Shell/terminal interface
- `app_placeholders` - Template apps for new features

**App Interface**:
```c
void app_name_create(lv_obj_t *parent);   // Initialize app UI
void app_name_destroy(void);              // Cleanup
```

#### Theme (`theme/theme_nordico.c/h`)
- Nordico color scheme implementation
- LVGL theme customization
- Visual identity for the OS

**Framework**: LVGL 9.x
- Object-oriented UI widgets (buttons, labels, screens, etc.)
- Event-driven programming model
- Lightweight graphics rendering (optimized for embedded)

---

## Data Flow & Interactions

### Startup Sequence

```
Platform Init (lv_port_pc_vscode)
    ↓
FreeRTOS Kernel Start
    ↓
LVGL Display & Input Driver Init
    ↓
firmware/runesos_fw_init()
    │
    ├─ Battery Service Start (task)
    ├─ Radio Service Start (task)
    ├─ Time Service Start (task)
    └─ HAL Table Initialization
    ↓
runesos-ui/runesos_init()
    │
    ├─ runesos_launcher_create() → Homescreen init
    ├─ runesos_session_init()   → State machine setup
    └─ runesos_statusbar_init() → Status indicators
    ↓
Main Loop (FreeRTOS Scheduler)
    └─ LVGL Task Tick (handles input, rendering, animations)
```

### User Interaction Flow

```
User Touch/Keyboard Input
    ↓
GT911 ISR → FreeRTOS Queue
    ↓
LVGL Input Device Handler
    ↓
App Event Handler
    ├─ Button Press: runesos_session→on_app_launch()
    └─ Other Input: App-specific handler
    ↓
App Logic Update
    ├─ Query services: battery_get(), radio_get(), etc.
    ├─ Update app state
    └─ Invalidate LVGL objects for redraw
    ↓
LVGL Rendering Task
    └─ Refresh display (480x640 RGB LCD via SPI/DBI)
```

### Service Update Cycle

```
FreeRTOS Timer Interrupt (e.g., 1 Hz)
    ↓
Time Service Task → runesos_time_get() → Update RTC HAL
    ↓
Battery Service Task → runesos_battery_get() → Poll ADC via HAL
    ↓
Radio Service Task → runesos_radio_get() → Poll WiFi/BLE stack
    ↓
Statusbar Task → Query services → Update UI widgets
    ↓
LVGL Render (next display cycle)
```

---

## Key Abstractions & Patterns

### 1. Hardware Abstraction (HAL)
- **Pattern**: Function pointer table (vtable)
- **Benefit**: Swap implementations without rebuilding core logic
- **Example**: HAL for real ESP32 vs. PC simulator

### 2. Service Layer
- **Pattern**: Facade over HAL + business logic
- **Benefit**: Reduces coupling between UI and hardware
- **Example**: `battery_service` provides higher-level battery queries

### 3. Plugin Architecture (Apps)
- **Pattern**: Standardized app interface (create/destroy)
- **Benefit**: New apps added via registry without modifying core
- **Example**: Add new app by implementing `app_newapp_create()` and registering

### 4. UI State Machine (Session)
- **Pattern**: Session tracks lockscreen → homescreen → app states
- **Benefit**: Clean lifecycle management for apps
- **Example**: `runesos_session` coordinates screen transitions

### 5. LVGL Object Hierarchy
- **Pattern**: Tree of LVGL objects (lv_obj_t)
- **Benefit**: Automatic event propagation, layout constraints, animations
- **Example**: Screen → Statusbar + Homescreen + Lockscreen

---

## Dependency Graph

```
runesos-ui/
    ├─ runesos_session.c  [depends on] runesos_homescreen, runesos_lockscreen, 
    │                                   app_registry
    ├─ runesos_homescreen.c [depends on] launcher, app_registry
    ├─ runesos_lockscreen.c [depends on] services (time, battery)
    ├─ runesos_statusbar.c [depends on] services (battery, radio, time)
    ├─ launcher/ [depends on] app_registry
    ├─ apps/ [depends on] theme, firmware/services
    └─ theme/ [no dependencies on other UI]

firmware/
    ├─ services/battery_service.c [depends on] hal_battery
    ├─ services/radio_service.c [depends on] hal_radio
    ├─ services/time_service.c [depends on] hal_time
    └─ hal/ [depends on] LVGL, FreeRTOS, ESP32 IDF, CPU drivers

lv_port_pc_vscode/
    ├─ src/main.c [depends on] LVGL, FreeRTOS, firmware, runesos-ui
    ├─ src/hal/ [implements] firmware/hal/ interfaces
    └─ FreeRTOS/ [submodule] Real-time kernel
```

---

## Development & Deployment Paths

### PC Simulator Build
```bash
cd lv_port_pc_vscode
cmake -S . -B build-runesos -G "MinGW Makefiles" -DUSE_FREERTOS=OFF
cmake --build build-runesos
./build-runesos/simulator
```
- Uses PC-native HAL implementations
- Allows UI/app development without hardware
- FreeRTOS optional (can use simple single-threaded loop)

### ESP32-S3 Firmware Build
```bash
pio run              # Compile
pio run -t upload    # Flash via USB
pio run -t monitor   # Serial console
```
- Real hardware HAL implementations
- Full FreeRTOS kernel active
- Optimized for PSRAM + flash constraints

---

## Quality & Design Principles

1. **Layered Architecture**: Clear separation (HAL → Services → Firmware → UI)
2. **Abstraction**: HAL allows swapping implementations
3. **Modularity**: Independent services, pluggable apps
4. **Minimal**: Lightweight design for embedded constraints (PSRAM, flash)
5. **Real-time**: FreeRTOS ensures responsive UI and timely sensor updates
6. **Portable**: UI/service logic independent of target platform
7. **Testable**: HAL abstraction enables mocking and simulator testing

---

## Extension Points

### Adding a New App
1. Create `runesos-ui/src/apps/app_newfeature.c`
2. Implement `app_newfeature_create(lv_obj_t *parent)`
3. Register in `runesos_app_registry.c`
4. Access services via `battery_get()`, `radio_get()`, etc.

### Adding a New Service
1. Create `firmware/services/newsvc_service.c/h`
2. Implement using HAL abstractions
3. Create FreeRTOS task if needed
4. Expose via header for UI consumption

### Supporting New Hardware
1. Extend `firmware/hal/hal_*.h` with new interface
2. Implement platform-specific version in `lv_port_pc_vscode/src/hal/`
3. Use function pointers in `runesos_hal_t` to delegate

---

## Current Limitations & Future Work

### Known Constraints
- **Memory**: 8MB PSRAM, 16MB flash (constrains app size, graphics assets)
- **Display**: Fixed 480x640 resolution (no scaling for different devices)
- **Performance**: Single-core ESP32-S3 (limited parallel processing)
- **Connectivity**: WiFi/BLE only (no cellular)

### Potential Enhancements
- [ ] Persistent app preferences (SPIFFS/LittleFS storage)
- [ ] Multi-tasking UI (concurrent app foreground/background)
- [ ] Over-the-air (OTA) firmware updates
- [ ] Custom themes/wallpapers (theme manager)
- [ ] Developer mode / debugging interface
- [ ] Gesture recognition (IMU-based interactions)
- [ ] Power profiling and optimization

---

## Summary Table

| Aspect | Technology | Purpose |
|--------|-----------|---------|
| **MCU** | ESP32-S3 | ARM Xtensa processor, wireless SoC |
| **RTOS** | FreeRTOS | Real-time task scheduling |
| **Graphics** | LVGL 9.x | Lightweight UI rendering |
| **Display Driver** | ST7701 | 480x640 RGB LCD control |
| **Touch Driver** | GT911 | Capacitive touch input |
| **Build Tool** | PlatformIO | Embedded project management |
| **Source Control** | Git | Version management (GitHub) |
| **Language** | C (C11) | Embedded standard |
| **Theme** | Nordico | Custom color scheme |

---

**Generated**: 2026-08-24  
**Project**: esp32-RunesOS  
**Owner**: rpinetti  
**License**: GPLv3
