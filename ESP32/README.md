# ESP32 Documentation

## ESP32 CAN Bus Setup

### Pin Assignments

The ESP32 requires an external CAN transceiver. I would recommend the use of the SN65HVD230.

CTX should be connected to GPIO21 (defined by TX_GPIO_NUM)
CRX should be connected to GPIO22 (defined by RX_GPIO_NUM)


------------------

## Build & Run

### ESP-IDF 

```sh
$ idf.py menuconfig
$ idf.py buid flash monitor
```


## References
1. [ESP32 API Reference CAN](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/twai.html)
