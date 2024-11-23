# [DeskHopL](../README.md) > [Docs](#)

## Shortcuts

- `CAPS_LOCK` jumps to the next PC
- `RIGHT ALT + RIGHT SHIFT + L` locks both screens
- `RIGHT ALT + RIGHT SHIFT + S` suspends both PCs
- `RIGHT ALT + RIGHT SHIFT + D` enables debug mode\*
- `RIGHT ALT + RIGHT SHIFT + R` request to reboot active board
- `RIGHT ALT + RIGHT SHIFT + Q` suspends active PC

\*the output will be shown on the `UART1 TX` pin.

## GPIO/Pins

### PICO A

| Description | GPIO | optional |
| ----------- | ---- | -------- |
| UART1 TX    | 4    | x        |
| UART1 RX    | 5    | x        |
| UART0 TX    | 12   |          |
| UART0 RX    | 13   |          |
| USB DP      | 14   |          |
| LED         | 25   |          |

### PICO B

| Description | GPIO | optional |
| ----------- | ---- | -------- |
| UART1 TX    | 4    | x        |
| UART1 RX    | 5    | x        |
| USB DP      | 14   |          |
| UART0 TX    | 16   |          |
| UART0 RX    | 17   |          |
| LED         | 25   |          |

## Suspending macOS

You can suspend your Mac with an Apple Keyboard by pressing `Option + Command + Media Eject`.
The Media Eject key is part of the consumer control descriptor, while Option & Command are included in the keyboard. If you send both reports properly, macOS will suspend.
But we need to make sure no other reports arrive on the Mac until it really has suspended. Otherwise it will just wake up again.

## Further links

- <https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf>
- <https://github.com/hathach/tinyusb>
- <https://github.com/hrvach/deskhop>
- <https://github.com/jfedor2/hid-remapper>
- <https://github.com/jfedor2/screen-hopper>
- <https://github.com/raspberrypi/debugprobe>
- <https://github.com/raspberrypi/picotool>
- <https://github.com/sekigon-gonnoc/Pico-PIO-USB>
- <https://openocd.org/doc/html/Debug-Adapter-Configuration.html>
- <https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html>
