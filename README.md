# [DeskHopL](#)

This project sprung by trying to make [hrvach](https://github.com/hrvach)s awesome project [deskhop](https://github.com/hrvach/deskhop) compatible with [my Logitech devices](https://github.com/hrvach/deskhop/issues/47).
This project is heavily based on his work, therefore all the credits belong to him and the deskhop community.

I started this project from scratch and just copied the very least code that is need for my opinionated setup.

## Features

- it's a MVP and has no batteries included
- we are using the `HID_PROTOCOL_REPORT` by default
- we are passing through received reports as-is (no re-mapping)
- your multi-monitor setup will just work as we don't alter the mouse\*
- HID device descriptors compatible with Logitech devices
- support for "Suspend both PCs" via shortcut (Linux/macOS)
- support for "Screensaver mode" by jiggling the mouse just one pixel

\*since we're keeping the relative mouse, moving your mouse to the next PC won't work. You have to press the `CAPS_LOCK` key to hop to the next PC.

## Documentation

For a more detailed documentation take a look [here](./docs/README.md).

## Building

_Note:_ don't forget to export `PICO_PIO_USB_PATH`, `PICO_TINYUSB_PATH` and `PICO_SDK_PATH` before building.

### Requirements

Make sure to have all requirements installed as described by [pico-sdk](https://github.com/raspberrypi/pico-sdk).

```sh
cd src
mkdir build
cmake ..
make
```

#### CMake Options

- `DH_DEBUG`: enables stdio-output on uart1
- `DH_PICO_2`: enables building for PICO 2 boards

## Device support

DeskHopL was tested with the following devices:

- Logitech MX Mechanical
- Logitech MX Master S3

Hub-Devices are currently not supported, see issues:

- <https://github.com/hathach/tinyusb/issues/1883>
- <https://github.com/hathach/tinyusb/issues/2195>
- <https://github.com/hathach/tinyusb/issues/2212>
