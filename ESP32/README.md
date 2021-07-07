# ESP32 Documentation

## ESP32 CAN Bus Setup

### Pin Assignments

The ESP32 requires an external CAN transceiver. I would recommend the use of the SN65HVD230.

CTX should be connected to GPIO21 (defined by TX_GPIO_NUM)
CRX should be connected to GPIO22 (defined by RX_GPIO_NUM)


------------------

## Build & Test

### Pull in submodules

```sh
$ git submodule update --init --recursive
```

### CMake building

```sh
$ mkdir build && cd build
$ cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
$ make
```

### Test

```sh
$ ./build/test/RPiCAN-tests
```


### Run
```sh
$ ./build/RPiCAN
```

## References
1. [ESP32 API Reference CAN](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/twai.html)
