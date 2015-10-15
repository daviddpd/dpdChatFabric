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
#include "args.h"
#include "cfConfig.h"
#include "cfPairConfig.h"


int main(int argc, char**argv)
{
	chatFabricConnection c;
	chatFabricPairing pair; 
	chatFabricConfig config;  
	msgbuffer b;
	unsigned char sessionNonce[crypto_secretbox_NONCEBYTES];

/*
	Initialization of all the needed fields.
	FIXME : Encapsulate this Initialization more.
*/
	bzero(&c,sizeof(c));	
	bzero(&pair,sizeof(pair));	
	bzero(&config,sizeof(config));	
	bzero(&b,sizeof(b));

	cfConfigInit(&config);
	
	
	b.length = -1;

	cfPairInit(&pair);
	chatPacket_calcNonce(1, &pair.mynonce, &sessionNonce );


	chatFabricAction a;
	
	chatFabric_args(argc, argv, &config, &a);	
	cfConfigRead(&config);
	if ( config.newconfigfile != NULL ) {
		cfConfigWrite(&config);	
	}
	
	

}