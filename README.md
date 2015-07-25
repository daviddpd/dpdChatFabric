# dpdChatFabric

David's light weight private messaging (and chat) fabric, with direct, relayed, and P2P/adhoc modes

## Requirements 

This is being developed on FreeBSD.  The follow ports are required:

* security/libsodium	
		
At the current time, I'm not developing or testing any other operating system. (But eventually will.)

* https://download.libsodium.org/doc/

I ripped out libucl, as it was too heavy weight for this application. I've moved to a binary encoded
config format, using the same idea that was used to encode the network package.

## Two different crypto libraries

libsodium proved to be difficult to port to ESP8266/xtensa-lx106.  So, I've also started to build a local one.  The local and libsodium uses in the chatFabric  are currently **not compatible**.

Libsodium implements both [ChaCha20 and Poly1305 for IETF protocols] [1] and [rfc7539] [2] of `AEAD_CHACHA20_POLY1305`, which will likely what I work towards.  However, for usage in ESP8266, I need a smaller implementation of  ChaCha20 with both 64 bit and 96 bit nonce support.  And ideally an AEAD utility wrapper too.


## Building 

### FreeBSD 

This will build with both GCC48 and clang-3.4.1, and xtensa-lx106-elf-gcc. 

		gmake HAVE_LOCAL_CRYPTO=1
		
or

		gmake HAVE_SODIUM=1

bsd-make won't work.  I must be using some gmake only conventions.		

### ESP8266 w/ using esp_iot_sdk_v1.2.0

I have not included the SDK files in this repo. So from `esp_iot_sdk_v1.2.0/examples/driver_lib`, the following files should be copied into the following locations.

```
	src/esp8266/driver/gpio16.c
	src/esp8266/driver/uart.c
	src/esp8266/include/driver/gpio16.h
	src/esp8266/include/driver/uart.h
	src/esp8266/include/driver/uart_register.h
```	

The current code size has extended beyond the two ROM 512k setup.  So, the include eagle.app.v6.full512.ld will map the irom0 segment starting at 0x10000, and stretches beyond the 0x40000 halfway point.   The configuration storaged is using the SDK's save with protect at offset 0x7a000.

The ESP8266 boots into Station mode, not AP (access point).  So, you need to also compile in the wifi configuration.  I didn't commit my networks SSID and password to git, so you need to create a `src/esp8266/include/driver/uart_register.h` with the contents:

```C
#define SSID "networkName"
#define SSID_PASSWORD "networkPassword"
```

If you are using the vagrant VirtualBox setup ... the makefile should just work. Jump into src/esp8266 and just `make`.

At writing, there is no way to clear the saved config.  Use esptool to write 16k of zeros starting at 0x7a000.

So somthing like this:

```shell
 dd if=/dev/zero of=16kblank.bin bs=16k count=1
 esptool.py --port /dev/cu.usbserial-AI025GVS write_flash 0x7a000 16kblank.bin 0x00000 firmware/0x00000.bin 0x10000 firmware/0x10000.bin
```


##  SPEC, PROTOCOL AND SOFTWARE IN HIGH DEGREE OF FLUX 

I'm still figuring out how to appropriately use encryption and will likely wildly change.

### Serialization Format

Generally, to keep things small footprint so they fit into small CPUs and controllers, a custom format is use.  The same format is using in config files, and flash config storage as well as the on-the-write format.  All encoding is network-byte order.  All data is proceed with a 1-byte tag. The tags can come in any order, with a few exceptions. Fields that are variable length like the random padding, the payload, and envelope must have lengths that preceed their content.  Otherwise, there is no way to tell when the content in the stream ends.

## Terminology 

Moving away from server and client terms.  Now, these are a "device" and a "controller".  A device is 
something that listens for commands and performs some action.  A controller is an end point that issues
commands or actions to devices.   

Currently, a device only listens and reacts. It's currently designed to be polled.  Current implementation 
only allows a device to be connected to a single controller. The controller can use multiple pair configuration
files to talk to multiple devices.

There will be a third actor, a gateway or relay, which will have the features of both devices and controllers.

## Usage

	device --config device.conf --ip 127.0.0.1 --port 1080 --debug
	
	controller --pairfile pair.conf --config client.conf --ip 127.0.0.1 --port 1080 --debug -m "Testing 1234"
	
	

# TODO

## Low Level Protocol 

* NONCE sync
* UUID to/from verification
* PIN/Password for use during pairing



[1]: https://tools.ietf.org/html/draft-agl-tls-chacha20poly1305-04
[2]: https://tools.ietf.org/html/rfc7539
