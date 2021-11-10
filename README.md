# Raspberry Pi Pico Logical Analyzer

## Getting started

- start vscode
- install recommended plugin
- click on `><` and select `Reopen in Container`
- configure & build
- ...

Plug in the RPi Pico (BOOTSEL) and copy the uf2 from the container to the RPi in a shell on the host.
Paths and container name may differ.

    docker cp <container>:/build/picologanal.uf2 /run/media/.../RPI-RP2/

## Hardware

* Raspberry Pi Pico
* 128x128 TFT with ST7735S (adafruit 2088)
* potentiometer

### Wiring


    19 (GP 14)    ---- Probe A
    20 (GP 15)    ---- Probe B

    22 (SPI0 CS)  ---- TCS
    24 (SPI0 SCK) ---- SCK
    25 (SPI0 TX)  ---- SI
    26 (GP 20)    ---- D/C
    27 (GP 21)    ---- RST

                      +
                      |
                      \
    31 (ADC0)  ---- ->/
                      \
                      |
                     GND

