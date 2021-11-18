# Raspberry Pi Pico Logical Analyzer

## Getting started

- start vscode
- install recommended plugin
- click on `><` and select `Reopen in Container`
- configure and build

Plug in the RPi Pico (BOOTSEL) and copy the uf2 from the container to the RPi in a shell on the host.
Paths and container name may differ.

    docker cp <container>:/build/picologanal.uf2 /run/media/.../RPI-RP2/

## Hardware

* Raspberry Pi Pico
* 128x128 TFT with ST7735S (adafruit 2088)
* 2 push buttons (+ 2 pull down resistors)
* LED (+ resistor)

### Wiring

#### Input Probes

    19 (GP 14)    ---- Probe A
    20 (GP 15)    ---- Probe B

#### Display

    22 (SPI0 CS)  ---- TCS
    24 (SPI0 SCK) ---- SCK
    25 (SPI0 TX)  ---- SI
    26 (GP 20)    ---- D/C
    27 (GP 21)    ---- RST

#### Buttons

     9 (GP  6)    ---- Left    ----/\/\/---- GND
    10 (GP  7)    ---- Right   ----/\/\/---- GND

#### Other

     1 (GP  0)    ---- Test Signal 1
     2 (GP  1)    ---- Test Signal 2

    29 (GP 22)    ----/\/\/---- Error LED ---- GND

