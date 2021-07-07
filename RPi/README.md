# Raspberry Pi Documentation


## RPi CAN Bus Setup

### Pin Assignments

MCP2515 Module Assignment:
```
PIN 2  (5V)	- SOLDERED WIRE
PIN 1  (3.3V)	- VCC
Pin 6  (GND)    - GND
GPIO10 (PIN 19)	- MOSI
GPIO09 (PIN 21) - MISO
GPIO25 (PIN 22) - INT
GPIO11 (PIN 23) - SCLK
GPIO08 (PIN 24) - CS
```

### Driver Installation

Add the following to /boot/config.txt file and then reboot the Raspberry Pi board.
```txt
dtoverlay=mcp2515-can0,oscillator=8000000,interrupt=25 
```
The oscillator parameter should be set to the actual crystal frequency found on the MCP2515 module. This frequency can change between modules. It is commonly either 16 or 8 MHz. For example if the MCP2515 CAN Bus module board has a 8MHz on-board crystal, set the above line to 8000000.

The interrupt parameter specifies the Raspberry PI GPIO pin number. This project has have connected the INT pin to GPIO25.

By default, the mcp2515 driver uses a maximum SPI frequency of 10MHz (as per the MCP2515 datasheet). You can also specify in the overlay an optional parameter **spimaxfrequency**, e.g. spimaxfrequency=2000000 to slow down the SPI clock to help with signal integrity issues – e.g. if you have flying leads, rather than a PCB!

After rebooting the driver should be loaded successfully. Check with:
```sh
$ dmesg | grep mcp251
[   21.673744] mcp251x spi0.0 can0: MCP2515 successfully initialized.
```
If you see a message like the following, check the power and wiring of the CAN controller.
```txt
[   21.705177] mcp251x spi0.0: Cannot initialize MCP2515. Wrong wiring?
[   21.705226] mcp251x spi0.0: Probe failed, err=19

```

When the driver is loaded, manually bring up the CAN interface with (bit rate can be as high as 1Mbit/s i.e. 1000000):
```sh
$ sudo ip link set can0 up type can bitrate 500000
```
```sh
$ ifconfig
can0: flags=193<UP,RUNNING,NOARP>  mtu 16
        unspec 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  txqueuelen 10  (UNSPEC)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```
```sh
$ ip -details link show can0
3: can0: <NOARP,UP,LOWER_UP,ECHO> mtu 16 qdisc pfifo_fast state UNKNOWN mode DEFAULT group default qlen 10
    link/can  promiscuity 0 
    can <TRIPLE-SAMPLING> state ERROR-ACTIVE restart-ms 100 
	  bitrate 500000 sample-point 0.750 
	  tq 250 prop-seg 2 phase-seg1 3 phase-seg2 2 sjw 1
	  mcp251x: tseg1 3..16 tseg2 2..8 sjw 1..4 brp 1..64 brp-inc 1
	  clock 4000000numtxqueues 1 gso_max_size 65536 gso_max_segs 65535

```
To automatically bring up the interface on boot, edit your /etc/network/interfaces file and add the following:
```
auto can0
iface can0 inet manual
    pre-up /sbin/ip link set can0 type can bitrate 500000 triple-sampling on restart-ms 100
    up /sbin/ifconfig can0 up
    down /sbin/ifconfig can0 down
```

### CAN-utils

Can-utils is a collection of debugging tools for use with the SocketCAN interface. It contains applications such as:

- **candump** - Dump CAN packets, display, filter and log to disk
- **canplayer** - Replay CAN log files.
- **cansend** - Send a single frame.
- **cangen** - Generate random traffic.
- **canbusload** - Display the current CAN bus utilisation.

To install can-utils: 
```sh
$ sudo apt install can-utils
```

### Virtual CAN (VCAN)

The Linux Virtual CAN driver can be used for testing purposes. To enable it:
```sh
$ sudo modprobe vcan
$ lsmod | grep vcan
vcan                    2670  0
```
```sh
$ sudo ip link add dev vcan0 type vcan
$ sudo ip link set up vcan0
$ ifconfig
vcan0: flags=193<UP,RUNNING,NOARP>  mtu 72
        unspec 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  txqueuelen 1000  (UNSPEC)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```
```sh
$ ls -l /sys/class/net/vcan*
lrwxrwxrwx 1 root root 0 Jul  2 14:57 /sys/class/net/vcan0 -> ../../devices/virtual/net/vcan0
```
Use **candump** and **cansend** from **can-utils** to test the virtual CAN
```sh
$ candump vcan0
```

Open up a second separate terminal window:
```txt
$ cansend vcan0 123#de.ad.be.ef
```
On the first terminal window observe the message from **candump** utility:
```sh
vcan0  123   [4]  DE AD BE EF
```

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
