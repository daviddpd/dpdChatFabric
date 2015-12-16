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
#include "util.h"
#include "cfConfig.h"
#include "cfPairConfig.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef ESP8266
#include "esp8266.h"
//#define __error (void)0
#endif


#ifndef ESP8266

void CP_ICACHE_FLASH_ATTR
chatFabric_consetup( chatFabricConnection *c,  char *ip, int port )
{

	int lowwater = 32;
	
	if ( c->type == SOCK_STREAM  &&  c->acceptedSocket == -1 && c->socket != -1 && c->bind == 1) {
		socklen_t len;
		c->acceptedSocket = accept(c->socket,(struct sockaddr *)&c->sockaddr,&len);
// 		CHATFABRIC_DEBUG_FMT(1,  
// 			"[DEBUG][%s:%s:%d] accpted socket: errno: %d  error: %s \n", 
// 			__FILE__, __FUNCTION__, __LINE__,  errno,strerror(errno) );
		return;
	} else if ( c->type == SOCK_STREAM  &&  c->socket != -1 && c->bind == 0) {
// 		CHATFABRIC_DEBUG_FMT(1,  
// 			"[DEBUG][%s:%s:%d] stream, socket, not bound \n", 
// 			__FILE__, __FUNCTION__, __LINE__,  errno,strerror(errno) );
		return;	
	} else if ( c->type == SOCK_DGRAM  &&  c->socket != -1 ) {
// 		CHATFABRIC_DEBUG_FMT(1,  
// 			"[DEBUG][%s:%s:%d] dgram, socket\n", 
// 			__FILE__, __FUNCTION__, __LINE__,  errno,strerror(errno) );
		return;	
	}
	// c->bind = doBind;
	c->acceptedSocket = -1;
	c->socket = -1;		
	c->socket=socket(AF_INET,c->type,0);

// 	CHATFABRIC_DEBUG_FMT(1,  
// 		"[DEBUG][%s:%s:%d] ERRNO: %d  Socket FD %d \n", 
// 		__FILE__, __FUNCTION__, __LINE__,  errno, c->socket );

	
	if (c->socket == -1 ) {
		return;
	}
	
	bzero( &(c->sockaddr),sizeof(c->sockaddr) );
	c->sockaddr.sin_family = AF_INET;
	if ( ip == 0 ) {
		c->sockaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	} else {
		c->sockaddr.sin_addr.s_addr=inet_addr(ip);
	}
	c->sockaddr.sin_port=htons(port);
	
	if ( c->type == SOCK_DGRAM ) {
		setsockopt(c->socket, SOL_SOCKET, SO_RCVLOWAT, &lowwater, sizeof(lowwater));
	}
	
	if ( c->type == SOCK_STREAM ) {
		int set = 1;
		setsockopt(c->socket, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
	}

	if ( c->bind ==  1 ) {
		bind(c->socket,(struct sockaddr *)&c->sockaddr,sizeof(c->sockaddr));
		if ( c->type == SOCK_STREAM ) 
		{
			listen(c->socket,1024);
			socklen_t len;
			c->acceptedSocket = accept(c->socket,(struct sockaddr *)&c->sockaddr,&len);
		}
	} else {
		if ( c->type == SOCK_STREAM ) {
			if ( connect(c->socket, (struct sockaddr *)&c->sockaddr, sizeof(c->sockaddr) ) < 0 ) {
// 			CHATFABRIC_DEBUG_FMT(1,  
// 				"[DEBUG][%s:%s:%d] ERRNO: %d  Socket Connect FD %d , %s \n", 
// 				__FILE__, __FUNCTION__, __LINE__,  errno, c->socket, strerror(errno) );			
			}

		}
	}
	
	return;
} 

#endif


enum chatFabricErrors CP_ICACHE_FLASH_ATTR
chatFabric_controller(chatFabricConnection *c, chatFabricPairing *pair, 
						chatFabricConfig *config, chatFabricAction *a , msgbuffer *b) 
{
	int n;
	socklen_t len;
	chatPacket *cp;
	msgbuffer mb;
	unsigned char * nullmsg = 0;
//	b->length = 0;		
	enum chatFabricErrors e;
#ifndef ESP8266
	chatFabric_consetup(c, config->ip, config->port);
	if ( c->socket == -1 ) {
//		CHATFABRIC_DEBUG(config->debug, "chatFabric connection setup failed" );
		return ERROR_SOCKET;
	}	
#endif

	if ( pair->state != STATE_PAIRED ) {
		cp = chatPacket_init (config, pair, CMD_PAIR_REQUEST,  nullmsg, 0,  0);
		chatPacket_encode ( cp, config, pair,  &mb, _CHATPACKET_ENCRYPTED, COMMAND);
	} else {
		if ( a->action == ACTION_APP_LIST ) {
			cp = chatPacket_init (config, pair, CMD_APP_LIST,  NULL, 0,  CMD_SEND_REPLY_TRUE);
		} else if ( a->action == ACTION_GET_CONFIG ) {
			cp = chatPacket_init (config, pair, CMD_CONFIG_GET,  NULL, 0,  CMD_SEND_REPLY_TRUE);
		} else if ( a->action == ACTION_SET_CONFIG ) {

		    CHATFABRIC_DEBUG_B2H(_GLOBAL_DEBUG, "cf Controller msg buffer",
        	                 (unsigned char*)b->msg, b->length  );
		
			cp = chatPacket_init (config, pair, CMD_CONFIG_SET,  b->msg, b->length,  CMD_SEND_REPLY_TRUE);

		    CHATFABRIC_DEBUG_B2H(_GLOBAL_DEBUG, "cf Controller cp payload",
        	                 (unsigned char*)cp->payload, cp->payloadLength  );

			free(b->msg);
			b->length = 0;
		} else if ( config->msg == 0 || config->msg == NULL ) {
			cp = chatPacket_init (config, pair, CMD_APP_MESSAGE,  NULL, 0,  CMD_SEND_REPLY_TRUE);
		} else { 
			cp = chatPacket_init (config, pair, CMD_APP_MESSAGE,  config->msg, strlen((const char *)config->msg),  CMD_SEND_REPLY_TRUE);
		}
		cp->action = a->action;
		cp->action_control = a->action_control;
		cp->action_type = a->action_type;
		cp->action_value = a->action_value;
		cp->action_length = a->action_length;
		chatPacket_encode ( cp, config, pair,  &mb, _CHATPACKET_ENCRYPTED, DATA);
	}
#ifdef ESP8266
		espconn_sent(c->conn, (uint8 *)mb.msg, mb.length);
#else
	len = sizeof(c->sockaddr);
	
	int retry = 3;
	do {
		if ( c->type == SOCK_STREAM ) {
			if (c->bind == 1 ) {
				n = write(c->acceptedSocket, mb.msg, mb.length);
			} else {
				n = write(c->socket, mb.msg, mb.length);		
			}
		} else {	
			n = sendto(c->socket, mb.msg, mb.length, 0, (struct sockaddr *)&(c->sockaddr), len);
		}
		if ( n == -1 ) {
			retry--;
			if ( c->type == SOCK_STREAM ) {
				close(c->socket);
			}			
			c->acceptedSocket = -1;
			c->socket = -1;
			chatFabric_consetup(c, config->ip, config->port);
			if ( c->socket == -1 ) {
				return ERROR_SOCKET;
			}
		} else {
			retry = 0;
		}
	} while ( retry );
#endif

	if ( config->debug ) {
		chatPacket_print(cp, OUT);
	}
						
//#ifndef ESP8266
	free(mb.msg);
//#endif
	mb.length = 0;
	chatPacket_delete(cp);
	
	int getAppList = 0;
	
	if (pair->state != STATE_PAIRED) {
		getAppList = 1;	
	}
	
	do { 
		e = chatFabric_device(c, pair, config, b);
	} while ( (e == ERROR_OK) && (pair->state != STATE_PAIRED)  );
	
/*	
	if ( getAppList ) {
		a->action = ACTION_APP_LIST;
		chatFabric_controller(c, pair, config, a, b);

	}
*/	
#ifndef ESP8266	
	//close (c->socket);
#endif
	return e;
	
}

enum chatFabricErrors CP_ICACHE_FLASH_ATTR
chatFabric_device(chatFabricConnection *c, chatFabricPairing *pair, chatFabricConfig *config, msgbuffer *b) 
{
/*
	CHATFABRIC_DEBUG_B2H(config->debug, "Device: pair shared key", (unsigned char *)&pair->sharedkey, crypto_box_PUBLICKEYBYTES );
	CHATFABRIC_DEBUG_B2H(config->debug, "Device: pair public key", (unsigned char *)&pair->publickey, crypto_box_PUBLICKEYBYTES );
	CHATFABRIC_DEBUG_B2H(config->debug, "Device:   nonce", (unsigned char *)&pair->nonce, crypto_secretbox_NONCEBYTES );

	CHATFABRIC_DEBUG_B2H(config->debug, "Device: mynonce", (unsigned char *)&pair->mynonce, crypto_secretbox_NONCEBYTES );
	CHATFABRIC_DEBUG_B2H(config->debug, "Device: my public",(unsigned char *)&config->publickey, crypto_box_PUBLICKEYBYTES );
	CHATFABRIC_DEBUG_B2H(config->debug, "Device: my private", (unsigned char *)&config->privatekey, crypto_box_PUBLICKEYBYTES );
*/

	int n, buffersize=1460;
	unsigned char *mesg;
	socklen_t len;
	chatPacket *cp, *cp_reply;
	msgbuffer mb;
	b->length = 0;	

	enum chatPacketCommands replyCmd;
	enum chatPacketPacketTypes cptype;
	enum chatFabricErrors e;
	
#ifdef ESP8266
	n = mbuff.length;
	mesg = mbuff.msg;
	int errno = 0;
#else
	if ( c->socket == -1 || ( c->acceptedSocket == -1 && c->bind == 1 )  ) {
		chatFabric_consetup(c, config->ip, config->port);
		if ( c->socket == -1 ) {
			CHATFABRIC_DEBUG(config->debug, "chatFabric connection setup failed" );
			return ERROR_SOCKET;
		}	
	}

	mesg=(unsigned char *)calloc(buffersize,sizeof(unsigned char));

//	CHATFABRIC_DEBUG(config->debug, "Waiting for Packet.\n\n" );

	len = sizeof(c->sockaddr);
	int retry = 3;
	do {
		if ( c->type == SOCK_STREAM ) { 
			if ( c->bind == 1 ) {
				n = read(c->acceptedSocket,mesg,buffersize);
			} else {
				n = read(c->socket,mesg,buffersize);
			}
			// n = recvfrom(c->acceptedSocket,mesg,buffersize,0,(struct sockaddr *)&(c->sockaddr),&len);
		} else {
			n = recvfrom(c->socket,mesg,buffersize,0,(struct sockaddr *)&(c->sockaddr),&len);
		}
		if ( n == -1 ) {
			retry--;
			if ( c->type == SOCK_STREAM ) {
				close(c->socket);
			}			
			c->acceptedSocket = -1;
			c->socket = -1;
			chatFabric_consetup(c, config->ip, config->port);
			if ( c->socket == -1 ) {
				return ERROR_SOCKET;
			}
		} else {
			retry = 0;
		}

	} while(retry);
#endif 	
	if ( n == -1 ) {
#ifndef ESP8266
		free(mesg); /* needed */ 
#endif
		CHATFABRIC_DEBUG(config->debug, "recvfrom returned an error" );
		CHATFABRIC_DEBUG_FMT(config->debug,  "ERRNO: %d, %s \n",   errno, strerror(errno) );
		return ERROR_OK;
	} 
	cp = chatPacket_init0 ();

	if ( chatPacket_decode (cp, pair, mesg, n, config ) != 0 ) {
		CHATFABRIC_DEBUG(config->debug, "chatPacket decoding failed." );
#ifndef ESP8266
		free(mesg);
#endif
//		chatPacket_delete(cp);
		cp->cmd = CMD_PACKET_DECRYPT_FAILED;

//		return ERROR_CHATPACKET_DECODING;		
	} else {
	
		CHATFABRIC_DEBUG(config->debug,  "chatPacket decoding successful ." );

#ifndef ESP8266
		if ( config->debug ) { 
			chatPacket_print (cp, IN);
		}
#endif

		if ( cp->payloadLength > 0 )
		{
			b->length = cp->payloadLength;
			b->msg=(unsigned char*)calloc(b->length,sizeof(unsigned char));		
			memcpy(b->msg, cp->payload, b->length);
		} else {
			b->length = 0;
		}

		if ( cp->action != ACTION_NULL ) {
			b->action = cp->action;
			b->action_control = cp->action_control;
			b->action_type = cp->action_type;
			b->action_value = cp->action_value;
			b->action_length = cp->action_length;

		}

#ifndef ESP8266		
		free(mesg);
#endif
	}
	
	cp_reply = chatPacket_init0 ();
	
	stateMachine ( config, cp, pair, cp_reply, &replyCmd, &e);
	if ( config->callback != NULL && pair->state == STATE_PAIRED && 
		( 
			cp->cmd == CMD_APP_MESSAGE || cp->cmd == CMD_APP_MESSAGE_ACK || 
			cp->cmd == CMD_APP_LIST || cp->cmd == CMD_APP_LIST_ACK 
		) 
	) {
		config->callback(config, cp, pair, cp_reply, &replyCmd);	
	}
	if ( replyCmd == CMD_SEND_REPLY_TRUE ) {

#ifndef ESP8266
		if ( config->debug ) {
			chatPacket_print (cp_reply, OUT);
		}
#endif

		switch (cp_reply->cmd) {
			case CMD_NONCE_SEND:
				cptype = NONCE;
			break;
			case CMD_PUBLICKEY_SEND:
				cptype = PUBLICKEY;
			break;
			case CMD_APP_MESSAGE:
			case CMD_APP_MESSAGE_ACK:
			case CMD_APP_REGISTER:
			case CMD_APP_LIST:
			case CMD_APP_LIST_ACK:
			case CMD_CONFIG_GET:
			case CMD_CONFIG_SET:
			case CMD_CONFIG_MESSAGE:			
				cptype = DATA;
			break;			
			default:
				cptype = COMMAND;
			break;
		}
		chatPacket_encode ( cp_reply, config, pair, &mb, _CHATPACKET_ENCRYPTED, cptype);
		// FIXME: Error check and handle sending error
#ifdef ESP8266
		espconn_sent(c->conn, (uint8 *)mb.msg, mb.length);
#else
	if ( c->type == SOCK_STREAM ) {
		if ( c->bind == 1 ) 
		{
		    n = write(c->acceptedSocket, mb.msg, mb.length);
		} else {
		    n = write(c->socket, mb.msg, mb.length);		
		}
		CHATFABRIC_DEBUG_FMT(config->debug,  
			"chatPacket TCP data sent. Bytes: %d  errno %d, socket %d %d  SOCK_TYPE : %d  %s", 
			n, errno,c->acceptedSocket, c->socket, c->type, strerror(errno)  );
	} else {	
		n = sendto(c->socket, mb.msg, mb.length, 0, (struct sockaddr *)&(c->sockaddr), len);	
		CHATFABRIC_DEBUG_FMT(config->debug,  
			"chatPacket UDP data sent. Bytes: %d  errno %d, socket %d %d  SOCK_TYPE : %d  %s", 
			n, errno,c->acceptedSocket, c->socket, c->type, strerror(errno)  );
	}
#endif

//#ifndef ESP8266
		free(mb.msg);
//#endif
		mb.length = 0;
		chatPacket_delete(cp_reply);
	} else {
		chatPacket_delete(cp_reply);	
	}
	
#ifndef ESP8266
	if ( ( c->type == SOCK_STREAM ) &&  (pair->state == STATE_PAIRED)  )
	{
		if ( c->acceptedSocket != -1 ) {
			close(c->acceptedSocket);
			c->acceptedSocket = -1;
		} else if ( c->socket != -1 ) {
		
//			close(c->socket);
//			c->socket = -1;
	
		}
		
		
//		CHATFABRIC_DEBUG_FMT(config->debug,  
//			"[DEBUG][%s:%s:%d] chatPacket TCP CLOSING Accepted Socket.  socket %d %d  SOCK_TYPE : %d  %s \n", 
//			__FILE__, __FUNCTION__, __LINE__,  c->acceptedSocket, c->socket, c->type, strerror(errno)  );
	}
#endif	
	chatPacket_delete(cp);
	return e;
}



void CP_ICACHE_FLASH_ATTR
stateMachine (chatFabricConfig *config, chatPacket *cp, chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd, enum chatFabricErrors *e)
{	

	chatFabricPairing  previous_state;
	
	enum chatPacketCommands RETVAL;
	int u0 = uuidCompare(&(cp->to.u0), &(config->uuid.u0));
	int u1 = uuidCompare(&(cp->to.u1), &(config->uuid.u1));
	
	if (  
		( ( u0 !=0 ) || ( u1 != 0 )  ) &&
		( cp->cmd != CMD_HELLO) && 
		( cp->cmd != CMD_HELLO_ACK) && 
		( cp->cmd != CMD_PAIR_REQUEST) 
	 ) 
	{
		*e = ERROR_INVAILD_DEST;
		RETVAL = CMD_SEND_REPLY_FALSE;
		return;
	}


	previous_state.state =  pair->state;
	previous_state.hasPublicKey = pair->hasPublicKey;

	uuidCopy( &cp->from.u0, &reply->to.u0);
	uuidCopy( &cp->from.u1, &reply->to.u1);
	uuidCopy( &config->uuid.u0, &reply->from.u0);
	uuidCopy( &config->uuid.u1, &reply->from.u1);
	
	RETVAL = CMD_SEND_REPLY_FALSE;

	switch ( cp->cmd ) {

		case CMD_PAIR_REQUEST_RESET:
			if ( pair->state != STATE_PAIRED ) {
				pair->state = STATE_UNCONFIGURED;
				reply->flags = pair->state;
				reply->cmd = CMD_PAIR_REQUEST;
				RETVAL = CMD_SEND_REPLY_TRUE;
			}
		break;
		case CMD_HELLO:
			reply->flags = pair->state;
			reply->cmd = CMD_HELLO_ACK;
			RETVAL = CMD_SEND_REPLY_TRUE;
		break;	

		case CMD_HELLO_ACK:
			reply->flags =  pair->state;
			reply->cmd = CMD_HELLO_ACK;
			RETVAL = CMD_SEND_REPLY_FALSE;
		break;	
		case CMD_PAIR_REQUEST:
			if ( pair->state == STATE_UNCONFIGURED ) {
				reply->flags = 0;
				reply->cmd = CMD_PAIR_REQUEST_ACK;
				pair->state = STATE_PUBLICKEY_SETUP;
				uuidCopy( &cp->from.u0, &pair->uuid.u0);
				uuidCopy( &cp->from.u1, &pair->uuid.u1);
				RETVAL = CMD_SEND_REPLY_TRUE;	
			}
		break;
		case CMD_PAIR_REQUEST_ACK:
			if ( pair->state == STATE_UNCONFIGURED ) {
				reply->flags = 0;
				reply->cmd = CMD_PUBLICKEY_REQUEST;
				pair->state = STATE_PUBLICKEY_SETUP;
				uuidCopy( &cp->from.u0, &pair->uuid.u0);
				uuidCopy( &cp->from.u1, &pair->uuid.u1);
				RETVAL = CMD_SEND_REPLY_TRUE;
			}
		break;
		case CMD_NONCE_REQUEST:
			if  ( pair->state == STATE_NONCE_SETUP ) {
				if ( pair->hasPublicKey == 1 ) {
					reply->flags = pair->hasNonce;
//					reply->nonce = pair->mynonce;
					memcpy(&(reply->nonce), &(pair->mynonce), crypto_secretbox_NONCEBYTES );
					reply->cmd = CMD_NONCE_SEND;
					pair->state = STATE_NONCE_SETUP;
					RETVAL = CMD_SEND_REPLY_TRUE;
				} else {
					reply->flags = 0;
					pair->state = STATE_UNCONFIGURED;
					reply->cmd = CMD_PAIR_REQUEST_RESET;
					RETVAL = CMD_SEND_REPLY_TRUE;
				}
			}
		break;
		case CMD_NONCE_ACK:
			if  ( pair->state == STATE_NONCE_SETUP ) {
					pair->state = STATE_PAIRED;
//					reply->cmd = CMD_APP_LIST;
					RETVAL = CMD_SEND_REPLY_FALSE;
//					a->action = (uint32_t )ACTION_GET;
//					a->action_control = 0;
//					a->action_value = 0;
			}		
		break;
		case CMD_NONCE_SEND:
			if  ( pair->state == STATE_NONCE_SETUP ) {
				memcpy( &(pair->nonce), &(cp->nonce), crypto_secretbox_NONCEBYTES );
				
				pair->hasNonce = 1;
				reply->flags = 0;
				if ( cp->flags == 1 ) {
					reply->cmd = CMD_NONCE_ACK;
					pair->state = STATE_PAIRED;				
				} else {
					reply->flags = pair->hasNonce;
					reply->cmd = CMD_NONCE_SEND;
//					reply->nonce = pair->mynonce;
					memcpy(&(reply->nonce), &(pair->mynonce), crypto_secretbox_NONCEBYTES );
					pair->state = STATE_NONCE_SETUP;
				}
				RETVAL = CMD_SEND_REPLY_TRUE;				
			}			
		break;
		case CMD_NONCE_RESET:
			RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_PUBLICKEY_ACK:
			if  ( pair->state == STATE_PUBLICKEY_SETUP ) {
				pair->state = STATE_NONCE_SETUP;
				reply->flags = pair->state;
				reply->cmd = CMD_NONCE_REQUEST;
				RETVAL = CMD_SEND_REPLY_TRUE;				
			}
		case CMD_PUBLICKEY_REQUEST:
			// FIXME: check UUIDs, Request PublicKey
			if  ( pair->state == STATE_PUBLICKEY_SETUP  ) {
				reply->flags = pair->hasPublicKey;
				reply->cmd = CMD_PUBLICKEY_SEND;
				memcpy( &(reply->publickey), &(config->publickey), crypto_box_PUBLICKEYBYTES );
				pair->state = STATE_PUBLICKEY_SETUP;
				RETVAL = CMD_SEND_REPLY_TRUE;	
			}
		break;
		case CMD_PUBLICKEY_SEND:
			if  ( pair->state == STATE_PUBLICKEY_SETUP ) {
				memcpy( &(pair->publickey), &(cp->publickey), crypto_box_PUBLICKEYBYTES );
				pair->hasPublicKey = 1;
				#ifdef HAVE_LOCAL_CRYPTO
				curve25519_donna((unsigned char *)&pair->sharedkey, (unsigned char *)&config->privatekey, (unsigned char *)&pair->publickey);
// 				if (config->debug) {
// 
// 					printf ( "   %24s: ", "Shared Key" );
//			util_debug_bin2hex("shared key", &pair->sharedkey, crypto_box_PUBLICKEYBYTES, 0, __FILE__, __FUNCTION__, __LINE__ );

// 					util_print_bin2hex((unsigned char *)pair->sharedkey, crypto_box_PUBLICKEYBYTES);
// 
// 					printf ( "   %24s: ", "private Key");
// 					util_print_bin2hex((unsigned char *)config->privatekey, crypto_box_PUBLICKEYBYTES);
// 
// 					printf ( "   %24s: ", "public key" );
// 					util_print_bin2hex((unsigned char *)pair->publickey, crypto_box_PUBLICKEYBYTES);
// 
// 				}
				
				#endif					

				// flags contains hasPublicKey? of remote 
				if ( cp->flags == 1 ) {
					reply->flags = pair->hasPublicKey;
					reply->cmd = CMD_PUBLICKEY_ACK;
					pair->state = STATE_NONCE_SETUP;
				} else {
					reply->flags = pair->hasPublicKey;
					reply->cmd = CMD_PUBLICKEY_SEND;
					memcpy( &(reply->publickey), &(config->publickey), crypto_box_PUBLICKEYBYTES );
				}
				RETVAL = CMD_SEND_REPLY_TRUE;
			}	
		break;
		
		case CMD_PUBLICKEY_RESET:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_VERIFY_SYN:
			if  ( ( pair->state == STATE_CONFIGURED  )  || ( pair->state == STATE_CONFIGURED_SYN  ) || ( pair->state == STATE_CONFIGURED_SYNACK  ) )
			{
				reply->flags = 0;
				reply->cmd = CMD_VERIFY_SYNACK;
				pair->state = STATE_CONFIGURED_SYN;
				RETVAL = CMD_SEND_REPLY_TRUE;	
			}		
		break;
		case CMD_VERIFY_SYNACK:
			if  ( 
				( pair->state == STATE_CONFIGURED  )  || 
				( pair->state == STATE_CONFIGURED_SYN  ) || 
				( pair->state == STATE_CONFIGURED_SYNACK  ) 
			)
			{
				reply->flags = 0;
				reply->cmd = CMD_VERIFY_ACK;
				pair->state = STATE_CONFIGURED_SYNACK;
				RETVAL = CMD_SEND_REPLY_TRUE;	
			}
		break;
		case CMD_VERIFY_ACK:
			if  ( ( pair->state == STATE_CONFIGURED  )  || ( pair->state == STATE_CONFIGURED_SYN  ) || ( pair->state == STATE_CONFIGURED_SYNACK  ) )
			{
				pair->state = STATE_PAIRED;
				reply->flags = 0;
				reply->cmd = CMD_VERIFY_ACK;
				pair->state = STATE_PAIRED;
				RETVAL = CMD_SEND_REPLY_TRUE;	
			}
		break;
				
		case CMD_APP_MESSAGE:
			if  (  pair->state == STATE_PAIRED  ) {
				if ( cp->flags == CMD_SEND_REPLY_TRUE ) {
					RETVAL = CMD_SEND_REPLY_TRUE;
				} else {
					RETVAL = CMD_SEND_REPLY_FALSE;		
				}
				reply->cmd = CMD_APP_MESSAGE_ACK;
			}
		break;
		case CMD_APP_MESSAGE_ACK:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_APP_REGISTER:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		
		case CMD_APP_LIST:
			if  (  pair->state == STATE_PAIRED  ) {
				RETVAL = CMD_SEND_REPLY_TRUE;
//				a->action = (uint32_t )ACTION_READ;
//				a->action_control = 0;
//				a->action_value = 0;
				reply->controlers = config->controlers;
				reply->numOfControllers = config->numOfControllers;
				
				reply->cmd = CMD_APP_LIST_ACK;
			}				
		break;
		
		case CMD_APP_LIST_ACK:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;

		case CMD_CONFIG_GET:
			reply->cmd = CMD_CONFIG_MESSAGE;
			msgbuffer configstr;
			
			_createConfigString(config, &configstr);
			
			reply->payload =  (unsigned char *)calloc(configstr.length,sizeof(unsigned char));
			memcpy( reply->payload, configstr.msg, configstr.length);
			reply->payloadLength = configstr.length;
			free(configstr.msg);
			
			CHATFABRIC_DEBUG_B2H(_GLOBAL_DEBUG, "Replay ConfigPayLoad", reply->payload, reply->payloadLength );
			RETVAL = CMD_SEND_REPLY_TRUE;	
		break;
		case CMD_CONFIG_SET:

	        cfConfigSetFromStr(config, cp->payload, cp->payloadLength);
	        
			CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "WiFI AP SSID %s", config->wifi_ap_ssid);
			CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "WiFI AP Switch %d", config->wifi_ap_switch);
	        cfConfigWrite(config);
	        
			reply->cmd = CMD_APP_MESSAGE_ACK;			
			RETVAL = CMD_SEND_REPLY_TRUE;			
		break;
		
		case CMD_CONFIG_MESSAGE:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_CONFIG_DELIGATE:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_CONFIG_PAIR:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_SEND_REPLY_FALSE:
			RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_SEND_REPLY_TRUE:
			RETVAL = CMD_SEND_REPLY_FALSE;
		break;		

		case CMD_PACKET_DECRYPT_FAILED:
			reply->cmd = CMD_FAIL;
//			RETVAL = CMD_SEND_REPLY_TRUE;
			RETVAL = CMD_SEND_REPLY_FALSE;
		break;

		case CMD_FAIL:
			RETVAL = CMD_SEND_REPLY_FALSE;
		break;		
		
		case CMD_INVAILD_CMD:
			RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		
		default:
			reply->flags = 0;
			reply->cmd = CMD_INVAILD_CMD;
			RETVAL = CMD_SEND_REPLY_TRUE;
		break;

	}

//#ifndef ESP8266	
	if ( previous_state.state == pair->state ) 
	{
//		CHATFABRIC_DEBUG_FMT(config->debug,
//			"[DEBUG][%s:%s:%d]  === [STATE] %-16s ==> %-16s %-36s \n", 
//			__FILE__, __FUNCTION__, __LINE__,
//			stateLookup(previous_state.state), stateLookup(pair->state), "STATE WAS UNCHANGED"  );
	} else {
		if ( pair->state == STATE_PAIRED ) {
#ifdef ESP8266
			config->pairfile = "1";
#endif
			config->hasPairs = 1;
			cfPairWrite(config, pair);
		}
//		CHATFABRIC_DEBUG_FMT(config->debug,
//			"[DEBUG][%s:%s:%d]  === [STATE] %-16s ==> %-16s %-36s \n",
//			__FILE__, __FUNCTION__, __LINE__,
//			stateLookup(previous_state.state), stateLookup(pair->state), "!!! STATE CHANGED !!" );
	}
//#endif

	*replyCmd = RETVAL;
	*e = ERROR_OK;
//	CHATFABRIC_DEBUG(config->debug, " return" );
}
