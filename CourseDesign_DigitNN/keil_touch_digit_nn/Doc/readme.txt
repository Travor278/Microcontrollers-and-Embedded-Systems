Keil Touch DigitNN Project
==========================

This directory is the Keil/STM32F103VE hardware project for the handwritten recognition course design.

Open:

  Project/RVMDK（uv5）/BH-F103.uvprojx

Target:

  DigitNN_Touch

Main documentation:

  ../README.md
  README.md

Hardware notes:

- CMSIS-DAP/DAPLink uses SWD for download and debug.
- USART1 is used for serial telemetry at 115200 8N1.
- The web dashboard reads POINT/STROKE/IMAGE/RESULT frames from the USB-to-serial port.
- User/digit_nn/generated contains the active model domain. Replace the whole generated set when switching between digit and letter firmware.

Generated build outputs:

- Output/ and Listing/ are Keil build products.
- *.uvguix.* is local Keil UI state.
- These files are ignored by git and can be regenerated locally.
