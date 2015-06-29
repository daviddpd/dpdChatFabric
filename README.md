# dpdChatFabric

David's light weight private messaging (and chat) fabric, with direct, relayed, and P2P/adhoc modes

## Requirements 

This is being developed on FreeBSD.  The follow ports are required:

* security/libsodium	
* textproc/libucl
		
At the current time, I'm not developing or testing any other operating system. (But eventually will.)

* https://download.libsodium.org/doc/
* https://github.com/vstakhov/libucl		


## Usage


	dpdChatFabric-client --config examples/client.conf --ip 127.0.0.1

	dpdChatFabric-server --config examples/server.conf

