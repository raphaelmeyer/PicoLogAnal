#include "pico_log_anal.h"

#include <hardware/pio.h>
#include <hardware/spi.h>

namespace {
Config const config{

    .display = {.clock = 18,
                .mosi = 19,
                .chip_select = 17,
                .data_command = 20,
                .reset = 21,
                .spi = spi0},
    .input = {.probe_base = 14, .pio = pio0},
    .control = {.scaler = 26},
    .test = {.output_a = 0, .output_b = 1}

};

PicoLogicalAnalyser logical_analylser{config};

} // namespace

int main() { logical_analylser.start(); }
