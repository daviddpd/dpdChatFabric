# dpdChatFabric

David's light weight private messaging (and chat) fabric, with direct, relayed, and P2P/adhoc modes

## Requirements 

This is being developed on FreeBSD.  The follow ports are required:

* security/libsodium	
		
At the current time, I'm not developing or testing any other operating system. (But eventually will.)

* https://download.libsodium.org/doc/

I ripped out libucl, as it was too heavy weight for this application. I've moved to a binary encoded
config format, using the same idea that was used to encode the network package.

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
