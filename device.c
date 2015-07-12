/* 
Copyright (c) 2015, David P. Discher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"


int main(int argc, char**argv)
{
	chatFabricConnection c;
	chatFabricPairing pair; 
	chatFabricConfig config;  
	msgbuffer b;
	uint32_t status2;
	unsigned char *tmp;

/*
	Initialization of all the needed fields.
	FIXME : Encapsulate this Initialization more.
*/
	bzero(&c,sizeof(c));	
	bzero(&pair,sizeof(pair));	
	bzero(&config,sizeof(config));	
	bzero(&b,sizeof(b));

	pair.state = STATE_UNCONFIGURED;
	pair.hasPublicKey = 0;
	
	uuid_create_nil ( &(pair.uuid.u0), &status2);
	uuid_create_nil ( &(pair.uuid.u1), &status2);
	pair.hasPublicKey = 0;
	pair.hasNonce = 0;
	b.length = -1;
	arc4random_buf(&(pair.mynonce), crypto_secretbox_NONCEBYTES);


	
	chatFabric_args(argc, argv, &config);	
	chatFabric_configParse(&config);
		
	while ( chatFabric_device(&c, &pair, &config,  &b) == ERROR_OK ) { 
	
		if ( b.length > 0 ) {
			tmp=calloc(b.length+1,sizeof(unsigned char));
			memcpy(tmp, b.msg, b.length);
			printf ( " === > Payload: %s \n", tmp ) ;
			free(tmp);
			free(b.msg);
			b.length = -1;
		}
	
	}
	

}

