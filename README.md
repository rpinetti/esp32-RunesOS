# RunesOS

> Um cyberdeck artesanal que vive dentro de uma lata de bala — um mini-celular
> de engenharia com identidade nórdica. Firmware para ESP32-S3 com display
> 480x640 RGB (ST7701), touch GT911 e teclado BLE.

[![License: GPL-3.0](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![Build](https://github.com/rpinetti/esp32-RunesOS/actions/workflows/build.yml/badge.svg)](https://github.com/rpinetti/esp32-RunesOS/actions/workflows/build.yml)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32--S3-orange)](https://platformio.org)

## ✨ O que é

O RunesOS é um sistema operacional minimalista para um cyberdeck DIY montado
dentro de uma lata de bala Barkley's. Não é uma ferramenta utilitária — é um
artefato de engenharia e diversão: um celular ultra-leve com lock screen, home
screen estilo celular, wallpaper e status bar, alimentado por ESP32-S3.

## 🛠️ Hardware

| Componente | Especificação |
|---|---|
| MCU | ESP32-S3 (8MB PSRAM, 16MB flash) |
| Display | 480x640 RGB, driver ST7701 |
| Touch | GT911 |
| Extras | RTC, sensor 6 eixos QMI8658, SD, speaker, carga de bateria |

## 🚀 Quick Start
```bash
# 1. Instale o PlatformIO
pip install platformio

# 2. Clone e compile
git clone https://github.com/rpinetti/esp32-RunesOS.git
cd esp32-RunesOS
pio run

# 3. Flash via USB
pio run -t upload
