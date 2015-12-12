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
#include "uuid_wrapper.h"
#include "cfConfig.h"
#include "cfPairConfig.h"

#include <assert.h>


void controllerCallBack(chatFabricConfig *config, chatPacket *cp,  chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd) 
{
//	printf ( "{controllerCallBack} %8s %12s %u %u\n",  actionLookup(cp->action), actionTypeLookup(cp->action_type), cp->action_control, cp->action_value  ) ;

	if ( cp->numOfControllers > 0 ) {
	
		// FIXME: possible memory leak.
		config->numOfControllers = cp->numOfControllers;
		config->controlers = cp->controlers;	
	
	}
	
	

	int i;
	for (i=0; i<config->numOfControllers; i++) 
	{
		if ( config->controlers[i].control == cp->action_control ) 
		{
			if (cp->action == ACTION_READ ) 
			{
				config->controlers[i].value = cp->action_value;
			}
				
		}

		printf ( "=== %10s: %4d %24s %4d \n", "Control", config->controlers[i].control, config->controlers[i].label, config->controlers[i].value );

	}

	return;
}

int main(int argc, char**argv)
{
	chatFabricConnection c;
	chatFabricPairing pair; 
	chatFabricConfig config;  
	msgbuffer b;
	unsigned char *tmp;

/*
	Initialization of all the needed fields.
	FIXME : Encapsulate this Initialization more.
*/
	bzero(&c,sizeof(c));	
	bzero(&pair,sizeof(pair));	
	bzero(&config,sizeof(config));	
	bzero(&b,sizeof(b));

	cfConfigInit(&config);

	pair.state = STATE_UNCONFIGURED;
	pair.hasPublicKey = 0;
	
	uuidCreateNil ( &(pair.uuid.u0));
	uuidCreateNil ( &(pair.uuid.u1));

	
	pair.hasPublicKey = 0;
	pair.hasNonce = 0;
	b.length = -1;
	arc4random_buf(&(pair.mynonce), crypto_secretbox_NONCEBYTES);
	bzero(&(pair.nullnonce), crypto_secretbox_NONCEBYTES);

	chatFabricAction a;
	bzero(&a,sizeof(a));

//	printf ( "====> ARGS Action <================== \n");
//	chatPacket_print_action2(&a);
//	printf ( "===================================== \n");

	chatFabric_args(argc, argv, &config, &a);	

	CHATFABRIC_DEBUG_B2H(config.debug, "action", (unsigned char *)&a.action, sizeof(a.action));
	
//	printf ( "====> ARGS Action <================== \n");
//	chatPacket_print_action2(&a);
//	printf ( "===================================== \n");

	cfConfigRead(&config);
	if ( config.newconfigfile != NULL ) {
		cfConfigWrite(&config);	
	}


	config.callback = &controllerCallBack;
	
	_GLOBAL_DEBUG = config.debug;


//	pair.uuid.u0 = config.to.u0;
//	pair.uuid.u1 = config.to.u1;
	
	if ( config.pairfile != NULL ) {
		printf ( "reading pair file\n");
		cfPairRead(&config, &pair);
	}


	c.type = config.type;
	c.socket = -1;
	c.acceptedSocket = -1;
	c.bind = 0;
	
	
/*	
	if ( strcmp ( "on", config.msg ) == 0 ) {	
		a.action = (uint32_t)ACTION_SET;
		a.action_type = (uint32_t)ACTION_TYPE_BOOLEAN;
		a.action_control = 0;
		a.action_value = 1;
			printf ( " === > setting on %u %u %u %u\n",  a.action, a.action_type, a.action_control,a.action_value  ) ;
	}
	
	if ( strcmp ( "off", config.msg ) == 0 ) {
		a.action = (uint32_t)ACTION_SET;
		a.action_type = (uint32_t) ACTION_TYPE_BOOLEAN;
		a.action_control = 0;
		a.action_value = 0;	
		printf ( " === > setting off %u %u %u %u\n",  a.action, a.action_type, a.action_control,a.action_value  ) ;
	}
*/

	if ( config.hasPairs ) {
		chatFabricAction applist;
		applist.action_length = 0;
		applist.action = ACTION_APP_LIST;	
		if ( chatFabric_controller(&c, &pair, &config, &applist,  &b) == ERROR_OK  ) {  }	
	} 

	close(c.socket);
	c.socket = -1;
	
	int i;
	for (i=0; i<config.numOfControllers; i++) 
	{
		if ( config.controlers[i].control == a.action_control ) 
		{
			a.action_type = config.controlers[i].type;
		}
	}

	if ( chatFabric_controller(&c, &pair, &config, &a,  &b) == ERROR_OK  ) { 
	
		if ( b.length > 0 ) {
			tmp=calloc(b.length+1,sizeof(unsigned char));
			memcpy(tmp, b.msg, b.length);
			printf ( " === > Payload: %s \n", tmp ) ;
			free(tmp);
			free(b.msg);
			b.length = -1;
		}
	
	}

	cfPairWrite(&config, &pair);
	
}

