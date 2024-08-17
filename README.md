# [DeskHopL](#)

This project sprung by trying to make [hrvach](https://github.com/hrvach)s awesome project [deskhop](https://github.com/hrvach/deskhop) compatible with [my Logitech devices](https://github.com/hrvach/deskhop/issues/47).
This project is heavily based on his work, therefore all the credits belong to him.

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

_Note:_ the cmake option `-DDH_DEBUG=1` enables stdio-output on uart1

## Device support

DeskHopL was tested with the following devices:

- Logitech MX Mechanical
- Logitech MX Master S3
