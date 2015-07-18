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

This will build with both GCC48 and clang-3.4.1, and xtensa-lx106-elf-gcc. 

		gmake HAVE_LOCAL_CRYPTO=1
		
or

		gmake HAVE_SODIUM=1
		
		


##  SPEC, PROTOCOL AND SOFTWARE IN HIGH DEGREE OF FLUX 

I'm still figuring out how to appropriately use encryption and will likely wildly change.


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
