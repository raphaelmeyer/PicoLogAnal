# Raspberry Pi Pico Logical Analyzer

## Getting started

- start vscode
- install recommended plugin
- click on `><` and select `Reopen in Container`
- configure & build
- ...

Plug in the RPi Pico (BOOTSEL) and copy the uf2 from the container to the RPi in a shell on the host.
Paths and container name may differ.

    docker cp toolchain-toolchain-1:/build/picologanal.uf2 /run/media/.../RPI-RP2/

