# Keil Touch DigitNN Project

This project is based on:

`1-书籍配套例程-F103VE指南者_20240202/29-电阻触摸屏—触摸画板`

## Open

Open this file in Keil uVision:

`Project/RVMDK（uv5）/BH-F103.uvprojx`

Target name:

`DigitNN_Touch`

## Download With DAPLink

In Keil:

1. `Options for Target`
2. `Debug`
3. Select `CMSIS-DAP Debugger`
4. Click `Settings`
5. Set `Port` to `SW`
6. Confirm the chip can be detected
7. Build and `Download`

## Pins

DAPLink SWD:

- `SWDIO` -> `PA13`
- `SWCLK` -> `PA14`
- `GND` -> `GND`
- `VTref/3V3` -> board `3.3V`
- `RST` -> `NRST` optional

Serial output uses the original example USART configuration.

## Run

1. Power on the board.
2. Calibrate the touch screen if prompted.
3. Write one digit in the white drawing area.
4. Tap `REC` in the lower-right button area.
5. LCD and USART print Perceptron/FNN recognition results.
6. Tap the clear button to clear both LCD drawing and recognition buffer.
