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

uint32_t controls[16];

void deviceCallBack(chatFabricConfig *config, chatPacket *cp,  chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd) 
{

	int i =0;
	unsigned char *tmp;
	if ( cp->payloadLength > 0 ) {

		tmp=calloc(cp->payloadLength,sizeof(unsigned char));
		memcpy(tmp, cp->payload, cp->payloadLength);
		printf ( " === > Payload: %s \n", tmp ) ;
	
	}

	printf ( " === >deviceCallBack  %u %u %u %u\n",  cp->action, cp->action_type, cp->action_control,cp->action_value  ) ;

	
	for (i=0; i<config->numOfControllers; i++) 
	{
		if ( config->controlers[i].control == cp->action_control ) 
		{
			if (cp->action == ACTION_GET ) 
			{
				reply->action = ACTION_READ;
				reply->action_control = cp->action_control;
				reply->action_type = config->controlers[i].type;
				reply->action_value = config->controlers[i].value;
				reply->action_length = 0;
		
		
			} else if (cp->action == ACTION_SET ) 
			{
				config->controlers[i].value = cp->action_value;

				reply->action = ACTION_READ;
				reply->action_control = cp->action_control;
				reply->action_type = config->controlers[i].type;
				reply->action_value = config->controlers[i].value;
				reply->action_length = 0;	
			}			
		}

		printf ( "=== %10s: %4d %24s %4d \n", "Control", config->controlers[i].control, config->controlers[i].label, config->controlers[i].value );

	}

}


int main(int argc, char**argv)
{
	chatFabricConnection c;
	chatFabricPairing pair; 
	chatFabricConfig config;  
	msgbuffer b;
	int i=0;

/*
	Initialization of all the needed fields.
	FIXME : Encapsulate this Initialization more.
*/
	bzero(&c,sizeof(c));	
	bzero(&pair,sizeof(pair));	
	bzero(&config,sizeof(config));	
	bzero(&b,sizeof(b));
	

/*	
	pair.state = STATE_UNCONFIGURED;
	pair.hasPublicKey = 0;
	uuid_create_nil ( &(pair.uuid.u0), &status2);
	uuid_create_nil ( &(pair.uuid.u1), &status2);
	pair.hasPublicKey = 0;
	pair.hasNonce = 0;
	arc4random_buf(&(pair.mynonce), crypto_secretbox_NONCEBYTES);
	bzero(&(pair.nullnonce), crypto_secretbox_NONCEBYTES);
*/	

	chatFabric_pair_init(&pair);

	b.length = -1;

	for (i=0; i<16; i++) {
		controls[i]=0;
	}

	chatFabricAction a;
	a.action_length = 0;

	chatFabric_args(argc, argv, &config, &a);	
	chatFabric_configParse(&config);
	
	config.callback = &deviceCallBack;
	c.type = config.type;
	c.socket = -1;
	c.acceptedSocket = -1;
	c.bind = 1;

	config.numOfControllers = 4;
	config.controlers = malloc(config.numOfControllers * sizeof(cfControl));

	i =	0;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].label = "switch0";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	

	i =1;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_DIMMER;
	config.controlers[i].value = 0;
	config.controlers[i].label = "Dimmer0";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 7;
	
	i =2;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_GAUGE;
	config.controlers[i].value = 0;
	config.controlers[i].label = "Gauge0 - Temp (F)";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= -200;
	config.controlers[i].rangeHigh= 300;

	i =3;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_DATA;
	config.controlers[i].value = 0;
	config.controlers[i].label = "Binary Data";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);


	do {		
		while ( chatFabric_device(&c, &pair, &config,  &b) == ERROR_OK ) { 
			free(b.msg);
			b.length = -1;
	
		}
	} while (1);
	

}

