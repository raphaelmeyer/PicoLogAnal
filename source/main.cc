#include "error.h"
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
    .control = {.left = 6, .right = 7},
    .test = {.output_a = 0, .output_b = 1},
    .error = {.light = 22}

};

Error error{config.error};
PicoLogicAnalyzer logic_analyzer{config};

} // namespace

int main() { logic_analyzer.start(); }
