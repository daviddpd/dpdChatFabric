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

#include "util.h"
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"

#ifdef ESP8266
//void inline free(void *x) { return; }
#include "esp8266.h"
#endif


const char * 
CP_ICACHE_FLASH_ATTR
actionTypeLookup (enum chatPacketActionsType tag) {

	switch (tag) {
		case ACTION_TYPE_NULL:
			return "NULL";
		break;
		case ACTION_TYPE_BOOLEAN:
			return "BOOLEAN";
		break;
		case ACTION_TYPE_DIMMER:
			return "DIMMER";
		break;
		case ACTION_TYPE_GAUGE:
			return "GAUGE";
		break;
		case ACTION_TYPE_DATA:
			return "DATA";
		break;
	}
	return "UNKNWON";	
}


const char * 
CP_ICACHE_FLASH_ATTR
actionLookup (enum chatPacketActions tag) {

	switch (tag) {
		case ACTION_NULL:
			return "NULL";
		break;
		case ACTION_GET:
			return "GET";
		break;
		case ACTION_SET:
			return "SET";
		break;
		case ACTION_READ:
			return "READ";
		break;
		case ACTION_APP_LIST:
			return "APP_LIST";
		break;
	}
	return "UNKNWON";
	
}



const char * 
CP_ICACHE_FLASH_ATTR
tagLookup (enum chatPacketTags tag) {

	switch (tag) {
		case cptag_NOOP:
			return "NOOP";
		break;
		case cptag_nonceLength:
			return "nonceLength";
		break;
		case cptag_nonce:
			return "nonce";
		break;
		case cptag_envelopeLength:
			return "envelopeLength";			
		break;
		case cptag_encryptedEnvelope:
			return "encryptedEnvelope";
		break;
		case cptag_envelope:
			return "envelope";
		break;
		case cptag_envelopeRandomPaddingLength:
			return "envelopeRandomPaddingLength";
		break;
		case cptag_envelopeRandomPaddingHigh:
			return "envelopeRandomPaddingHigh";
		break;
		case cptag_envelopeRandomPaddingLow:
			return "envelopeRandomPaddingLow";
		break;
		case cptag_to0:
			return "uuid_to0";
		break;
		case cptag_to1:
			return "uuid_to1";
		break;
		case cptag_from0:
			return "uuid_from0";		
		break;
		case cptag_from1:
			return "uuid_from1";		
		break;
		case cptag_flags:
			return "flags";		
		break;
		case cptag_payloadLength:
			return "payloadLength";		
		break;
		case cptag_payloadRandomPaddingLength:
			return "payloadRandomPaddingLength";		
		break;
		case cptag_payloadRandomPaddingHigh:
			return "payloadRandomPaddingHigh";
		break;
		case cptag_payloadRandomPaddingLow:
			return "payloadRandomPaddingLow";
		break;
		case cptag_encryptedPayload:
			return "encryptedPayload";
		break;
		case cptag_payload:
			return "payload";
		break;
		case cptag_publickeyId:
			return "publickeyId";
		break;
		case cptag_publickey:
			return "publickey";
		break;
		case cptag_mac:
			return "mac";
		break;
		case cptag_serial:
			return "serial";
		break;
		case cptag_cmd:
			return "cmd";
		break;
		case cptag_action:
			return "action";
		break;
		case cptag_action_type:
			return "action_type";
		break;
		case cptag_action_value:
			return "action_value";
		break;
		case cptag_action_length:
			return "action_length";
		break;
		case cptag_action_data:
			return "action_data";
		break;
		case cptag_action_control:
			return "action_control";
		break;
		case cptag_app_controls:
			return "app_controls";
		break;
		case cptag_app_control_i:
			return "app_control_i";
		break;
		case cptag_app_control_type:
			return "app_control_type";
		break;
		case cptag_app_control_value:
			return "app_control_value";
		break;
		case cptag_app_control_rangeLow:
			return "app_control_rangeLow";
		break;
		case cptag_app_control_rangeHigh:
			return "app_control_rangeHigh";
		break;
		case cptag_app_control_labelLength:
			return "app_control_labelLength";
		break;
		case cptag_app_control_label:
			return "app_control_label";
		break;
		case cptag_app_control_data:
			return "app_control_data";
		break;
	 	 	 	 		
	}
	return "UNKNOWN";
}


const char * 
CP_ICACHE_FLASH_ATTR
stateLookup (enum chatPacketStates state) {

	switch (state) {
		case STATE_UNCONFIGURED:
		 return "STATE_UNCONFIGURED";
		break;
		case STATE_PUBLICKEY_SETUP:
		 return "STATE_PUBLICKEY_SETUP";
		break;
		case STATE_PAIRING_SETUP:
		 return "STATE_PAIRING_SETUP";
		break;
		case STATE_NONCE_SETUP:
		 return "STATE_NONCE_SETUP";
		break;
		case STATE_CONFIGURED:
		 return "STATE_CONFIGURED";
		break;
		case STATE_CONFIGURED_SYN:
		 return "STATE_CONFIGURED_SYN";
		break;
		case STATE_CONFIGURED_SYNACK:
		 return "STATE_CONFIGURED_SYNACK";
		break;
		case STATE_PAIRED:
		 return "STATE_PAIRED";
		break;
	}
	return "UNKNWON";
}



const char * 
CP_ICACHE_FLASH_ATTR
cmdLookup (enum chatPacketCommands cmd) {
	switch (cmd) {	
	case CMD_INVAILD_CMD:
		return "CMD_INVAILD_CMD";
	break;
	case CMD_HELLO:
		return "CMD_HELLO";
	break;
	case CMD_PAIR_REQUEST:
		return "CMD_PAIR_REQUEST";
	break;
	case CMD_PAIR_REQUEST_ACK:
		return "CMD_PAIR_REQUEST_ACK";
	break;
	case CMD_HELLO_ACK:
		return "CMD_HELLO_ACK";
	break;
	case CMD_NONCE_REQUEST:
		return "CMD_NONCE_REQUEST";
	break;
	case CMD_NONCE_SEND:
		return "CMD_NONCE_SEND";
	break;
	case CMD_NONCE_RESET:
		return "CMD_NONCE_RESET";
	break;
	case CMD_PUBLICKEY_REQUEST:
		return "CMD_PUBLICKEY_REQUEST";
	break;
	case CMD_PUBLICKEY_SEND:
		return "CMD_PUBLICKEY_SEND";
	break;
	case CMD_PUBLICKEY_RESET:
		return "CMD_PUBLICKEY_RESET";
	break;
	case CMD_VERIFY_SYN:
		return "CMD_VERIFY_SYN";
	break;
	case CMD_VERIFY_SYNACK:
		return "CMD_CMD_VERIFY_SYNACK";
	break;
	case CMD_VERIFY_ACK:
		return "CMD_VERIFY_ACK";
	break;
	case CMD_APP_MESSAGE:
		return "CMD_APP_MESSAGE";
	case CMD_APP_MESSAGE_ACK:
		return "CMD_APP_MESSAGE_ACK";
	break;
	case CMD_APP_REGISTER:
		return "CMD_APP_REGISTER";
	break;
	case CMD_APP_LIST:
		return "CMD_APP_LIST";
	break;
	case CMD_CONFIG_MESSAGE:
		return "CMD_CONFIG_MESSAGE";
	break;
	case CMD_CONFIG_DELIGATE:
		return "CMD_CONFIG_DELIGATE";
	break;
	case CMD_CONFIG_PAIR:
		return "CMD_CONFIG_PAIR";
	break;
	case CMD_SEND_REPLY_FALSE:
		return "CMD_SEND_REPLY_FALSE";
	break;
	case CMD_SEND_REPLY_TRUE:
		return "CMD_SEND_REPLY_TRUE";
	break;
	case CMD_PAIR_REQUEST_RESET:
		return "CMD_PAIR_REQUEST_RESET";
	break;
	case CMD_NONCE_ACK:
		return "CMD_NONCE_ACK";
	break; 
	case CMD_PUBLICKEY_ACK:
		return "CMD_PUBLICKEY_ACK";
	break;
	case CMD_APP_LIST_ACK:
		return "CMD_APP_LIST_ACK";
	break;
	
	}
	return "UNKNOWN";


}

void CP_ICACHE_FLASH_ATTR
chatPacket_calcNonce(uint32_t serial, unsigned char * nonce, unsigned char * sessionNonce )
{

	uint32_t last4Bytes = crypto_secretbox_NONCEBYTES - 4;
	uint32_t ninc = 0, ninc2 =0;
	
	memcpy( (unsigned char *)sessionNonce, (unsigned char *)nonce, crypto_secretbox_NONCEBYTES );
	memcpy ( &ninc, sessionNonce+last4Bytes, 4);

#ifdef ESP8266
	ninc2 = be32dec((void *)&ninc);
	ninc2 += serial;
	ninc = 0;
	ninc = be32dec((void *)&ninc2);
#else
	ninc2 = ntohl(ninc);
	ninc2 += serial;
	ninc = 0;
	ninc = htonl(ninc2);
#endif

	memcpy (  sessionNonce+last4Bytes, &ninc, 4);

}
/*
void CP_ICACHE_FLASH_ATTR
chatPacket_tagDataEncoder( enum chatPacketTagData type, unsigned char *b, uint32_t *i, unsigned char tag,  uint32_t value, unsigned char*s, uint32_t len, uuid_cp *uuid)
{

	uint32_t x = *i;
	uint32_t ni = 0;
				CHATFABRIC_DEBUG_FMT(0,  
					"[DEBUG][%s:%s:%d]  Encode(%4d) %-20s %02x %4d \n",
					__FILE__, __FUNCTION__, __LINE__, x, tagLookup(tag), tag , value);
	
	memcpy(b+x, &tag, 1);
	++x;
	if ( type == CP_INT32 ) {
#ifdef ESP8266	
		ni = ntohl(value);
#else 
		ni = htonl(value);
#endif
		memcpy(b+x, &ni, 4);
		x += 4;		
	} else if (  type == CP_DATA8 ) {	
		memcpy(b+x, s, len);
		x += len;
	} else if (  type == CP_UUID ) {
		uuidToBytes(b+x, uuid);
		x += 16;
	}

	*i=x;
}
*/


chatPacket*
CP_ICACHE_FLASH_ATTR
chatPacket_init0 (void) {
	chatPacket * cp;
	unsigned char h, l, hp, lp; // high / low envelope and payload padding
#ifdef ESP8266
	unsigned char _align[4];
#endif

	int i=0;
#ifndef ESP8266	

	cp=(chatPacket *)calloc(1,sizeof(chatPacket));
	if  ( cp == 0 ) {
		return NULL;
	}
#else 
	for (i=0; i<16; i++) {
		if (cpStatus[i] == -1) {
			cp = &(cpArray[i]);
			cpStatus[i] = 1;
			cp->cpindex = i;
			break;
		}
	}
	if ( cp == NULL ) {
		return 0;
	}		
#endif
	cp->cmd = 0;
	cp->flags = 0;
	//cp->nonce = 0;
	cp->envelopeLength = 0;
	cp->wasEncrypted = -1;
	cp->serial = 0;
	cp->controlers = NULL;
	cp->numOfControllers = 0;

	cp->envelopeRandomPaddingLength = 0;
	for (i=0; i<16; i++) {
		cp->envelopeRandomPadding[i]=0;
		cp->payloadRandomPadding[i]=0;
	}

	uuidCreateNil(&(cp->to.u0));
	uuidCreateNil(&(cp->to.u1));
	uuidCreateNil(&(cp->from.u0));
	uuidCreateNil(&(cp->from.u1));

	cp->payloadLength =0;

	for (i=0; i<crypto_box_PUBLICKEYBYTES; i++) {
		cp->publickey[i] = 0;
	}

	cp->payloadRandomPaddingLength = 0;
	
	cp->payload = 0;
	
	h =  arc4random_uniform(15) + 1;
	l = 16 - h;
	cp->envelopeRandomPaddingLength = (h << 4) | l;

	arc4random_buf((unsigned char *)&(cp->envelopeRandomPadding), 16);

	hp =  arc4random_uniform(15) + 1;
	lp = 16 - hp;
	cp->payloadRandomPaddingLength = (hp << 4) | lp;
	arc4random_buf((unsigned char *)&(cp->payloadRandomPadding), 16);


	cp->action = ACTION_NULL;
	cp->action_control = 0;
	cp->action_type = ACTION_TYPE_NULL;
	cp->action_value = 0;
	cp->action_length = 0;
	
	return cp;


}


chatPacket*
CP_ICACHE_FLASH_ATTR
chatPacket_init (chatFabricConfig *config, chatFabricPairing *pair, enum chatPacketCommands cmd, unsigned char *payload, uint32_t len, uint32_t flags) {

	chatPacket * cp;
	uuid_tuple *to = &(pair->uuid);
	unsigned char h, l, hp, lp; // high / low envelope and payload padding

#ifndef ESP8266	

	cp=(chatPacket *)calloc(1,sizeof(chatPacket));
	if  ( cp == 0 ) {
		return NULL;
	}
#else 
	int i;
	for (i=0; i<16; i++) {
		if (cpStatus[i] == -1) {
			cp = &(cpArray[i]);
			cpStatus[i] = 1;
			cp->cpindex = i;
			break;
		}
	}
	if ( cp == NULL ) {
		return 0;
	}		
#endif

	cp->payload = (unsigned char*)calloc(len,sizeof(unsigned char) );
	if ( cp->payload == 0 ) {
		free(cp);
		return 0;	
	}

	cp->serial = 0;

	cp->controlers = NULL;
	cp->numOfControllers = 0;

	h =  arc4random_uniform(15) + 1;
	l = 16 - h;
	cp->envelopeRandomPaddingLength = (h << 4) | l;
	arc4random_buf((unsigned char *)&(cp->envelopeRandomPadding), 16);

	hp =  arc4random_uniform(15) + 1;
	lp = 16 - hp;
	cp->payloadRandomPaddingLength = (hp << 4) | lp;
	arc4random_buf((unsigned char *)&(cp->payloadRandomPadding), 16);

	uuidCopy( &to->u0, &cp->to.u0);
	uuidCopy( &to->u1, &cp->to.u1);

	uuidCopy(&(config->uuid.u0), &(cp->from.u0));
	uuidCopy(&(config->uuid.u1), &(cp->from.u1));

	cp->cmd = cmd;
	cp->flags = flags;
	cp->wasEncrypted = -1;
	cp->payloadLength = len;	
	memcpy(cp->payload, payload, len);

	cp->action = ACTION_NULL;
	cp->action_control = 0;
	cp->action_type = ACTION_TYPE_NULL;
	cp->action_value = 0;
	cp->action_length = 0;

		
	return cp;

}

void
CP_ICACHE_FLASH_ATTR
chatPacket_delete (chatPacket* cp) {
//	CHATFABRIC_DEBUG(1, "start " );
	free(cp->payload);
	if ( cp->action_length > 0 ) {
		free(cp->action_data);
	}

#ifndef ESP8266
	free(cp);
#else
	cpStatus[cp->cpindex] = -1;
#endif
//	CHATFABRIC_DEBUG(1, "return " );

}

void
CP_ICACHE_FLASH_ATTR
chatPacket_encode (chatPacket *cp, chatFabricConfig *config, chatFabricPairing *pair, msgbuffer *ob, int encrypted, enum chatPacketPacketTypes packetType) {
	uint32_t p_length =0, e_length=0, ob_length=0, encrypted_envolopeLength=0;
	unsigned long long p_length_encrpyted=0;
	uint32_t i, ix;
//	uint32_t ni, ni2;

	unsigned char h,l;
		
	unsigned char *envelope;
	unsigned char *envelope_encrypted;
	unsigned char *payload;
	unsigned char *payload_encrypted;

	ob_length = 1+4 + 1+4 + 4*(1+16);	
	
	if (packetType == PUBLICKEY ){
		ob_length += crypto_box_PUBLICKEYBYTES + 1;		
	}

	if (packetType == DATA ) {

		h = 0;
		l = 0;

		h = cp->payloadRandomPaddingLength & 0xF0;
		h = h >> 4;	

		l = cp->payloadRandomPaddingLength  & 0x0F;	
		i = 0;
	
		p_length = 1+cp->payloadLength  + 2 + 1+16 + 1+4 + 1;
	
		if ( cp->action != 0 ) {
			p_length += 5 + 5 + 5 + 5 + 5;
			if ( cp->action_length > 0 ) {
				p_length +=	1 + cp->action_length;
			}
		}

		
		if ( cp->numOfControllers > 0 ) {
			p_length+=5;
			for (ix=0; ix<cp->numOfControllers; ix++)  {

				p_length+=(5*5); // 5x 32-bit ints plus tags,plus 2 for leading tag+value
				if ( cp->controlers[ix].type == ACTION_TYPE_DATA  &&  cp->controlers[ix].value>0 ) {
					p_length+=cp->controlers[ix].value + 1;
				}
				if (cp->controlers[ix].labelLength > 0 ) {
					p_length+=cp->controlers[ix].labelLength + 1 + 5; // label+tag, labelLength+tag
				}
		CHATFABRIC_DEBUG_FMT(config->debug,  
			"[DEBUG][%s:%s:%d]  SumLength(%4d) %4d \n",
			__FILE__, __FUNCTION__, __LINE__, ix, p_length);
			}
		}
	
		if ( encrypted == _CHATPACKET_ENCRYPTED ) {
			payload=(unsigned char*)calloc(p_length,sizeof(unsigned char));		
		
			p_length_encrpyted = p_length + crypto_secretbox_MACBYTES; 
			payload_encrypted=(unsigned char*)calloc(p_length_encrpyted,sizeof(unsigned char));	
			
			chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_payloadLength, p_length, NULL, 0, NULL);			
			
			ob_length+=p_length_encrpyted + 1+4 + 1;
				
		} else {
			payload=(unsigned char*)calloc(p_length,sizeof(unsigned char));
			ob_length+=p_length;		
		}
		

		chatPacket_tagDataEncoder ( CP_DATA8, payload, &i, cptag_payloadRandomPaddingLength, 0, &cp->payloadRandomPaddingLength, 1, NULL);
		chatPacket_tagDataEncoder ( CP_DATA8, payload, &i, cptag_payloadRandomPaddingHigh, 0, (unsigned char *)&cp->payloadRandomPadding, h, NULL);
		
		if ( cp->action != 0 ) {
			chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_action, cp->action, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_action_control, cp->action_control, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_action_type, cp->action_type, NULL, 0, NULL);

			chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_action_value, cp->action_value, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_action_length, cp->action_length, NULL, 0, NULL);
			if ( cp->action_length > 0 ) {
				chatPacket_tagDataEncoder ( CP_DATA8, payload, &i, cptag_action_data, 0, cp->action_data, cp->action_length, NULL);	
			}

		}
		
		if ( cp->numOfControllers > 0 ) {
			chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_app_controls, cp->numOfControllers, NULL, 0, NULL);

			for (ix=0; ix<cp->numOfControllers; ix++)  {			
				chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_app_control_i, cp->controlers[ix].control, NULL, 0, NULL);
				chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_app_control_type, cp->controlers[ix].type, NULL, 0, NULL);
				chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_app_control_value, cp->controlers[ix].value, NULL, 0, NULL);
				chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_app_control_rangeLow, cp->controlers[ix].rangeLow, NULL, 0, NULL);
				chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_app_control_rangeHigh, cp->controlers[ix].rangeHigh, NULL, 0, NULL);

				if (  cp->controlers[ix].labelLength > 0 && cp->controlers[ix].label != NULL) {
					chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_app_control_labelLength, cp->controlers[ix].labelLength, NULL, 0, NULL);
					chatPacket_tagDataEncoder ( CP_DATA8, payload, &i, cptag_app_control_label, 0, (unsigned char *)cp->controlers[ix].label, cp->controlers[ix].labelLength, NULL);
				}

				if ( cp->controlers[ix].type == ACTION_TYPE_DATA && cp->controlers[ix].value > 0 && cp->controlers[ix].data != NULL) {
					chatPacket_tagDataEncoder ( CP_DATA8, payload, &i, cptag_app_control_data, 0, (unsigned char *)cp->controlers[ix].data, cp->controlers[ix].value, NULL);				
				}

			}		
		}
		
				
		chatPacket_tagDataEncoder ( CP_DATA8, payload, &i, cptag_payload, 0, cp->payload, cp->payloadLength, NULL);
				
		chatPacket_tagDataEncoder ( CP_DATA8, payload, &i, cptag_payloadRandomPaddingLow, 0, (unsigned char *)&(cp->payloadRandomPadding[h]), l, NULL);

		if ( i != p_length ) {	
			printf ( " WARNING = PAYLOAD => index didn't match p_length! %u != %u\n", i, p_length );	
		}
	
		if ( encrypted == _CHATPACKET_ENCRYPTED ) {
			pair->serial++;
			cp->serial = pair->serial;
			unsigned char *sessionNonce = (unsigned char*)calloc(crypto_secretbox_NONCEBYTES,sizeof(unsigned char));
			chatPacket_calcNonce(cp->serial, (unsigned char *)&(pair->nonce), sessionNonce);
			
			
			#ifdef HAVE_SODIUM
			crypto_box_easy(
				payload_encrypted, 
				payload, p_length, 
				(const unsigned char *)sessionNonce,
				(unsigned char *)&(pair->publickey), 
				(unsigned char *)&(config->privatekey)
			);
			#endif
			
			#ifdef HAVE_LOCAL_CRYPTO
			memcpy ( payload_encrypted, payload, p_length );
			s20_crypt((uint8_t*)&pair->sharedkey, S20_KEYLEN_256, sessionNonce, 0, payload_encrypted, p_length);
			poly1305_auth(payload_encrypted+p_length, payload_encrypted, p_length, (unsigned char *)&pair->sharedkey);
			
			if (config->debug) {
				printf ( "   %24s: ", "MAC Calculated" );
				util_print_bin2hex((unsigned char *)payload_encrypted+p_length, crypto_secretbox_MACBYTES);
				printf ( "   %24s (%8llu): ", "encrypted payload",  p_length_encrpyted);
				util_print_bin2hex((unsigned char *)payload_encrypted, p_length_encrpyted);
				printf ( "   %24s (%8d): ", "plaintext payload",  p_length);
				util_print_bin2hex((unsigned char *)payload, p_length);


				printf ( "   %24s: ", "sessionNonce" );
				util_print_bin2hex((unsigned char *)sessionNonce, crypto_secretbox_NONCEBYTES);
				printf ( "   %24s: ", "nonce" );
				util_print_bin2hex((unsigned char *)pair->nonce, crypto_secretbox_NONCEBYTES);
				printf ( "   %24s: ", "shared key" );
				util_print_bin2hex((unsigned char *)&pair->sharedkey, crypto_box_PUBLICKEYBYTES);

			}
			
			#endif

			
			free(sessionNonce);
			free(payload);
			/*
			crypto_aead_chacha20poly1305_encrypt(
				payload_encrypted, &p_length_encrpyted,
				(const unsigned char *)payload, 
				(unsigned long long)p_length,
				NULL, 0,
				NULL, 
				(const unsigned char *)&cp->payloadNonce, 
				(const unsigned char *)&sharedkey_payload);
			*/

		}

	}

	/* ****** Envelope  *****************************************************  */	

	if (packetType == NONCE ) {

		e_length =  2 + 1+16 + 1+crypto_secretbox_NONCEBYTES + 1;
		
		if ( encrypted == _CHATPACKET_ENCRYPTED ) {
			encrypted_envolopeLength = e_length + crypto_box_SEALBYTES;		
		} else {
			encrypted_envolopeLength = e_length;
		
		}

		h = cp->envelopeRandomPaddingLength & 0xF0;
		h = h >> 4;
		l = cp->envelopeRandomPaddingLength & 0x0F;	
		i = 0;
		
		envelope = (unsigned char*)calloc(e_length,sizeof(unsigned char));


		chatPacket_tagDataEncoder ( CP_DATA8, envelope, &i, cptag_envelopeRandomPaddingLength, 0, &(cp->envelopeRandomPaddingLength), 1, NULL);
		chatPacket_tagDataEncoder ( CP_DATA8, envelope, &i, cptag_envelopeRandomPaddingHigh, 0, (unsigned char *)&(cp->envelopeRandomPadding), h, NULL);

		chatPacket_tagDataEncoder ( CP_DATA8, envelope, &i, cptag_nonce, 0, (unsigned char *)&(pair->mynonce), crypto_secretbox_NONCEBYTES, NULL);

		chatPacket_tagDataEncoder ( CP_DATA8, envelope, &i, cptag_envelopeRandomPaddingLow, 0,(unsigned char *)&(cp->envelopeRandomPadding[h]), l, NULL);
		
		if (config->debug){ 
			CHATFABRIC_DEBUG(config->debug, " ======> Plaintext Envelope : " );
			util_print_bin2hex (envelope, e_length );		
		}
		
	
		if ( encrypted == _CHATPACKET_ENCRYPTED ) {
			envelope_encrypted =  (unsigned char*)calloc(encrypted_envolopeLength,sizeof(unsigned char));
			#ifdef HAVE_SODIUM
			crypto_box_seal(envelope_encrypted, envelope, e_length, (unsigned char *)&(pair->publickey));
			#endif
			
			#ifdef HAVE_LOCAL_CRYPTO
			memcpy ( envelope_encrypted, envelope, e_length );			
			s20_crypt((uint8_t*)&pair->sharedkey, S20_KEYLEN_256, pair->nullnonce, 0, envelope_encrypted, e_length);
			poly1305_auth(envelope_encrypted+e_length, envelope_encrypted, e_length, (unsigned char *)&pair->sharedkey);
			if (config->debug) {
				printf ( "   %24s: ", "MAC Calculated" );
				util_print_bin2hex((unsigned char *)envelope_encrypted+e_length, crypto_secretbox_MACBYTES);
				printf ( "   %24s (%8d): ", "encrypted envelope",  e_length);
				util_print_bin2hex((unsigned char *)envelope_encrypted, e_length);

				printf ( "   %24s (%8d): ", "Nonce",  crypto_secretbox_NONCEBYTES);
				util_print_bin2hex((unsigned char *)pair->nullnonce, crypto_secretbox_NONCEBYTES);
			}
			
			#endif			
			if (config->debug){ 
				CHATFABRIC_DEBUG(config->debug, " ======> encrypted Envelope : " );
				util_print_bin2hex (envelope_encrypted, encrypted_envolopeLength );		
			}

			cp->envelopeLength = encrypted_envolopeLength;
			ob_length+=encrypted_envolopeLength + 1 + 1+4 ; // + cptag_encryptedEnvelope + cptag_envelopeLength + cp->envelopeLength
			free(envelope);
		} else {
			ob_length+=encrypted_envolopeLength + 1+4;			
		}
	}
	
	/*
		Add in Serial Number 
	*/
	ob_length+=5;
	ob->length = ob_length;
	ob->msg = (unsigned char*)calloc(ob->length,sizeof(unsigned char));;
	i=0;
		
	chatPacket_tagDataEncoder ( CP_INT32, ob->msg, &i, cptag_cmd, cp->cmd, NULL, 0, NULL);
	chatPacket_tagDataEncoder ( CP_INT32, ob->msg, &i, cptag_serial, cp->serial, NULL, 0, NULL);

	chatPacket_tagDataEncoder ( CP_INT32, ob->msg, &i, cptag_flags, cp->flags, NULL, 0, NULL);

	chatPacket_tagDataEncoder ( CP_UUID, ob->msg, &i, cptag_to0, 0, NULL, 0,  &cp->to.u0);
	chatPacket_tagDataEncoder ( CP_UUID, ob->msg, &i, cptag_to1, 0, NULL, 0,  &cp->to.u1);
	chatPacket_tagDataEncoder ( CP_UUID, ob->msg, &i, cptag_from0, 0, NULL, 0,  &cp->from.u0);
	chatPacket_tagDataEncoder ( CP_UUID, ob->msg, &i, cptag_from1, 0, NULL, 0,  &cp->from.u1);

	if (packetType == PUBLICKEY ) {
		chatPacket_tagDataEncoder ( CP_DATA8, ob->msg, &i, cptag_publickey, 0,(unsigned char *)&(cp->publickey), crypto_box_PUBLICKEYBYTES, NULL);		
	} 

	if (packetType == NONCE ) {		
		chatPacket_tagDataEncoder ( CP_INT32, ob->msg, &i, cptag_envelopeLength, encrypted_envolopeLength, NULL, 0, NULL);
		cp->envelopeLength = encrypted_envolopeLength;
		if ( encrypted == _CHATPACKET_ENCRYPTED ) {
			chatPacket_tagDataEncoder ( CP_DATA8, ob->msg, &i, cptag_encryptedEnvelope, 0, envelope_encrypted, encrypted_envolopeLength, NULL);
			free(envelope_encrypted);		
		} else {
			chatPacket_tagDataEncoder ( CP_DATA8, ob->msg, &i, cptag_envelope, 0, envelope, e_length, NULL);
			free(envelope);		
		}
	}

	if (packetType == DATA ) {
		if ( encrypted == _CHATPACKET_ENCRYPTED ) {
			chatPacket_tagDataEncoder ( CP_INT32, ob->msg, &i, cptag_payloadLength, p_length_encrpyted, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_DATA8, ob->msg, &i, cptag_encryptedPayload, 0, payload_encrypted, p_length_encrpyted, NULL);
			free(payload_encrypted);		
		} else {
			chatPacket_tagDataEncoder ( CP_INT32, ob->msg, &i, cptag_payloadLength, cp->payloadLength, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_DATA8, ob->msg, &i, cptag_encryptedPayload, 0, payload, p_length, NULL);
			free(payload);		
		}
	} 

	if ( i != ob_length ) {	
		printf ( " WARNING = OB Length => index didn't match e_length! %u != %u\n", i, ob_length );	
	}
	
	if (config->debug) {
		CHATFABRIC_DEBUG(config->debug, " encoded Packet : " );
		util_print_bin2hex (ob->msg, ob->length );		
	}
		
}


int 
CP_ICACHE_FLASH_ATTR
chatPacket_decode (chatPacket *cp,  chatFabricPairing *pair, unsigned char *b, const int len, chatFabricConfig *config) {
	uint32_t ni=0, i=0, length = 0, curControler=0;
	unsigned char c=0, h=0, l=0, hp=0, lp = 0;
	unsigned char *decrypted=0;
	int ret;
	unsigned char * mac;
	
	int config_debug = config->debug;

	if (config_debug) {
		CHATFABRIC_DEBUG(config_debug, " decoding Packet : " );
		util_print_bin2hex (b, len);		
	}
	
	
//	const int len2 = len;

	while (i<len) {
		memcpy(&c, b+i, 1);
		++i;
				CHATFABRIC_DEBUG_FMT(config_debug,  
					"[DEBUG][%s:%s:%d]  Decode : %-20s %02x ( %02x-%02x-%02x-%02x ) (i<len : %4d %4d ) \n",
					__FILE__, __FUNCTION__, __LINE__, tagLookup(c), c, b[i], b[i+1], b[i+2], b[i+3], i, len );
		
		switch (c){
			case cptag_cmd:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->cmd = ntohl(ni);
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
					
			break;
			case cptag_serial:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->serial = ntohl(ni);					
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_nonce:
				memcpy(cp->nonce, b+i, crypto_secretbox_NONCEBYTES);
				i += crypto_secretbox_NONCEBYTES;				
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			
			case cptag_envelopeLength:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->envelopeLength = ntohl(ni);
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;				
			case cptag_encryptedEnvelope:
					cp->wasEncrypted = cp->wasEncrypted | 0x02;
					decrypted=(unsigned char*)calloc(cp->envelopeLength - crypto_box_SEALBYTES,sizeof(unsigned char));

					#ifdef HAVE_SODIUM
					ret = crypto_box_seal_open(
						decrypted, 
						b+i, cp->envelopeLength, 
						(unsigned char *)&(config->publickey), 
						(unsigned char *)&(config->privatekey));
					#endif 
						
					#ifdef HAVE_LOCAL_CRYPTO
					mac=(unsigned char*)calloc(crypto_secretbox_MACBYTES,sizeof(unsigned char) );					
					poly1305_auth(mac,  b+i, cp->envelopeLength - crypto_box_SEALBYTES, (unsigned char *)&pair->sharedkey);
					ret = poly1305_verify(mac, b+i+cp->envelopeLength-crypto_secretbox_MACBYTES);
					if ( ret == 1 ) {					
						memcpy ( decrypted, b+i, cp->envelopeLength-crypto_box_SEALBYTES );
						s20_crypt((uint8_t*)&pair->sharedkey, S20_KEYLEN_256, pair->nullnonce, 0, decrypted, cp->envelopeLength - crypto_box_SEALBYTES);
						ret = 0;
					} else {
						ret = -1;
					}
					#endif

					if ( ret == 0)
					{
						i+=cp->envelopeLength;
						free(mac);	
						if ( chatPacket_decode (cp, pair, decrypted, cp->envelopeLength - crypto_box_SEALBYTES, config) == 0 ) {
							free(decrypted);
							return 0;
						}
					} else {
						free(decrypted);
						CHATFABRIC_DEBUG(config_debug,  " ===> Decryption (envelope) Failed \n" );
						CHATFABRIC_DEBUG_FMT(config_debug,  
							"[DEBUG][%s:%s:%d]  i = %4d,   el = %4d \n",
							__FILE__, __FUNCTION__, __LINE__,  i, cp->envelopeLength);
						if (config_debug) {
							printf ( "   %24s (%8d): ", "MAC Calculated" , crypto_secretbox_MACBYTES);
							util_print_bin2hex((unsigned char *)mac, crypto_secretbox_MACBYTES);
							printf ( "   %24s (%8d): ", "MAC Stream",  cp->envelopeLength);
							util_print_bin2hex((unsigned char *)b+i+cp->envelopeLength-crypto_secretbox_MACBYTES, crypto_secretbox_MACBYTES);
							printf ( "   %24s (%8d): ", "encrypted envelope",  cp->envelopeLength);
							util_print_bin2hex((unsigned char *) b+i, cp->envelopeLength);
						}
						free(mac);	
					return -1;
					}
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_envelopeRandomPaddingLength:
				memcpy(&(cp->envelopeRandomPaddingLength), b+i, 1);
				i+=1;
				h = cp->envelopeRandomPaddingLength & 0xF0;				
				h = h >> 4;
				l = cp->envelopeRandomPaddingLength & 0x0F;				
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_envelopeRandomPaddingHigh:
				if ( h == 0 ) {
					// zero length is invalid padding.
					//CHATFABRIC_DEBUG(config_debug, " ===> cptag_envelopeRandomPaddingHigh == 0 \n");
					return -1;
				}
				memcpy(&(cp->envelopeRandomPadding), b+i, h);
				i+=h;
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;			
			case cptag_envelopeRandomPaddingLow:
				if ( l == 0 ) {
					// zero length is invalid padding.
					//CHATFABRIC_DEBUG(config_debug, " ===> cptag_envelopeRandomPaddingLow == 0 \n");
					return -1;
				}
				memcpy(&(cp->envelopeRandomPadding[h]), b+i, l);
				i+=l;
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_to0:
				uuidFromBytes(b+i, &cp->to.u0);
//				util_print_bin2hex((unsigned char *)b+i, 16);
				i+=16;
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_to1:
				uuidFromBytes(b+i, &cp->to.u1);
//				util_print_bin2hex((unsigned char *)b+i, 16);
				i+=16;
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_from0:
				uuidFromBytes(b+i, &cp->from.u0);
//				util_print_bin2hex((unsigned char *)b+i, 16);
				i+=16;
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_from1:
				uuidFromBytes(b+i, &cp->from.u1);
//				util_print_bin2hex((unsigned char *)b+i, 16);
				i+=16;
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_flags:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->flags = ntohl(ni);			
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			
			case cptag_payloadLength:
				memcpy(&ni, b+i, 4);
				i+=4;
				length = ntohl(ni);
				if ( length > cp->payloadLength ) {
					free(cp->payload);
					cp->payload=(unsigned char*)calloc(length,sizeof(unsigned char));
				}				
				cp->payloadLength = length;
				
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;			
			case cptag_payloadRandomPaddingLength:
				memcpy(&(cp->payloadRandomPaddingLength), b+i, 1);
				i+=1;
				hp = cp->payloadRandomPaddingLength & 0xF0;
				hp = hp >> 4;
				lp = cp->payloadRandomPaddingLength & 0x0F;			
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_payloadRandomPaddingHigh:
				if ( hp == 0 ) {
					// zero length is invalid padding.
					//CHATFABRIC_DEBUG(config_debug, " ===> cptag_payloadRandomPaddingHigh == 0 \n");
					return -1;
				}
				memcpy(&(cp->payloadRandomPadding), b+i, hp);
				i+=hp;
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;			
			
			case cptag_payloadRandomPaddingLow:
				if ( lp == 0 ) {
					// zero length is invalid padding.
					//CHATFABRIC_DEBUG(config_debug, " ===> cptag_payloadRandomPaddingLow == 0 \n");
					return -1;
				}
				memcpy(&(cp->envelopeRandomPadding[hp]), b+i, lp);
				i+=lp;
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;			
			case cptag_encryptedPayload:
				cp->wasEncrypted = cp->wasEncrypted | 0x01;			
				//free(cp->payload);				
				unsigned char *sessionNonce = (unsigned char*)calloc(crypto_secretbox_NONCEBYTES,sizeof(unsigned char));
				chatPacket_calcNonce(cp->serial, (unsigned char *)&(pair->mynonce), sessionNonce);
				
				#ifdef HAVE_LOCAL_CRYPTO
				mac=(unsigned char*)calloc(crypto_secretbox_MACBYTES,sizeof(unsigned char) );
				
				poly1305_auth(mac,  b+i,  cp->payloadLength - crypto_secretbox_MACBYTES, (unsigned char *)&pair->sharedkey);
				ret = poly1305_verify(mac, b+i+cp->payloadLength-crypto_secretbox_MACBYTES);
				free(mac);
//				ret =1 ;									
					CHATFABRIC_DEBUG(config_debug, " ===> Decrypting  (payload) \n");
					if (config_debug) {
						printf ( "   %24s: ", "sessionNonce" );
						util_print_bin2hex((unsigned char *)sessionNonce, crypto_secretbox_NONCEBYTES);
						printf ( "   %24s: ", "nonce" );
						util_print_bin2hex((unsigned char *)pair->mynonce, crypto_secretbox_NONCEBYTES);
						printf ( "   %24s: ", "shared key" );
						util_print_bin2hex((unsigned char *)&pair->sharedkey, crypto_box_PUBLICKEYBYTES);
					}
									
				if ( ret == 1 ) {					
					decrypted=(unsigned char*)calloc(cp->payloadLength - crypto_secretbox_MACBYTES,sizeof(unsigned char));
					memcpy ( decrypted, b+i, cp->payloadLength - crypto_secretbox_MACBYTES );
					s20_crypt((uint8_t*)&pair->sharedkey, S20_KEYLEN_256, sessionNonce, 0, decrypted, cp->payloadLength - crypto_secretbox_MACBYTES);
					ret = 0;
				} else {
					ret = -1;
				}
				#endif
				#ifdef HAVE_SODIUM
				decrypted=(unsigned char*)calloc(cp->payloadLength - crypto_secretbox_MACBYTES,sizeof(unsigned char));	
				ret = crypto_box_open_easy(
						decrypted, 
						(unsigned char *)b+i, cp->payloadLength, 
						(const unsigned char *)sessionNonce,  
						(unsigned char *)&(pair->publickey), 
						(unsigned char *)&(config->privatekey) );
				#endif
				free(sessionNonce);
				if (ret == 0 )
				{
					i+=cp->payloadLength;
					if ( chatPacket_decode (cp, pair, decrypted, cp->payloadLength - crypto_secretbox_MACBYTES, config) != 0 ) {
						free(decrypted);
						return -1;
					} else {
						free(decrypted);
					}					
				} else {
					free(decrypted);
					CHATFABRIC_DEBUG(config_debug, " ===> Decryption (payload) Failed \n");
					if (config_debug) {
						printf ( "   %24s: ", "MAC Calculated" );
						util_print_bin2hex((unsigned char *)mac, crypto_secretbox_MACBYTES);
						printf ( "   %24s: ", "MAC Stream" );
						util_print_bin2hex((unsigned char *)b+i+cp->payloadLength, crypto_secretbox_MACBYTES);
					}
					
					return -1;
				}

				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_payload:
				memcpy(cp->payload, b+i, cp->payloadLength);
				i+=cp->payloadLength;		
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_publickey:
				memcpy(&(cp->publickey), b+i, crypto_box_PUBLICKEYBYTES);
				i+=crypto_box_PUBLICKEYBYTES;
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_envelope:
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_action:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->action = ntohl(ni);			
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_action_control:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->action_control = ntohl(ni);			
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_action_type:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->action_type = ntohl(ni);			
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_action_value:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->action_value = ntohl(ni);			
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_action_length:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->action_length = ntohl(ni);
				if ( cp->action_length > 0 ) {
					cp->action_data=(unsigned char*)calloc(cp->action_length,sizeof(unsigned char));
				}				
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_action_data:
				memcpy(cp->action_data, b+i, cp->action_length);
				i+=cp->payloadLength;			
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_app_controls:
				memcpy(&ni, b+i, 4);
				i+=4;
				curControler = -1;
				cp->numOfControllers = ntohl(ni);

				CHATFABRIC_DEBUG_FMT(config_debug,  
					"[DEBUG][%s:%s:%d]  App Controls Num(%4d) CurControler(%4d) \n",
					__FILE__, __FUNCTION__, __LINE__, cp->numOfControllers,  curControler);

				cp->controlers = (cfControl*)malloc(cp->numOfControllers * sizeof(cfControl));
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
				
			break;
			
			case cptag_app_control_i:
				curControler++;
				memcpy(&ni, b+i, 4);
				i+=4;
				CHATFABRIC_DEBUG_FMT(config_debug,  
					"[DEBUG][%s:%s:%d]  App Controls Num(%4d) CurControler(%4d) \n",
					__FILE__, __FUNCTION__, __LINE__, ntohl(ni),  curControler);
				cp->controlers[curControler].control = ntohl(ni);
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			
			case cptag_app_control_type:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->controlers[curControler].type = ntohl(ni);
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_app_control_value:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->controlers[curControler].value = ntohl(ni);
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_app_control_rangeLow:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->controlers[curControler].rangeLow = ntohl(ni);
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_app_control_rangeHigh:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->controlers[curControler].rangeHigh = ntohl(ni);
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_app_control_labelLength:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->controlers[curControler].labelLength = ntohl(ni);
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_app_control_label:
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
				cp->controlers[curControler].label =  (char *)calloc(cp->controlers[curControler].labelLength, sizeof(char));
				memcpy(cp->controlers[curControler].label, b+i, cp->controlers[curControler].labelLength);
				i+=cp->controlers[curControler].labelLength;
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
			case cptag_app_control_data:
				cp->controlers[curControler].data = (char *)calloc(cp->controlers[curControler].value, sizeof(char));
				memcpy(&ni, b+i, cp->controlers[curControler].value);
				i+=cp->controlers[curControler].value;
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
			break;
						
			default:
//				printf ( " == BAD CHAT PACKET (%02x) =>> Last 6 bytes %02x %02x %02x %02x %02x %02x \n ", c, b[i-2],  b[i-1], b[i], b[i+1], b[i+2], b[i+3]);
				CHATFABRIC_DEBUG_FMT(config_debug,  
					"[DEBUG][%s:%s:%d]  BAD CHAT PACKET (%02x) =>> \n",
					__FILE__, __FUNCTION__, __LINE__,  c );
				//assert(0);
				CHATFABRIC_DEBUG_FMT(config_debug, "[DEBUG][%s:%s:%d] %8s : %-20s (i<len : %4d %4d ) \n", __FILE__, __FUNCTION__, __LINE__,  "CASE" , tagLookup(c), i, len );
				return -1;
				++i;
			break;
		}
CHATFABRIC_DEBUG_FMT(config_debug,  
	"[DEBUG][%s:%s:%d] WHILELOOP:  i<len : %4d %4d \n",
	__FILE__, __FUNCTION__, __LINE__, i, len );
	}

CHATFABRIC_DEBUG_FMT(config_debug,  
	"[DEBUG][%s:%s:%d] EXIT:  i<len : %4d %4d \n",
	__FILE__, __FUNCTION__, __LINE__, i, len );
	
	return 0;

}

void CP_ICACHE_FLASH_ATTR
chatPacket_print_action (chatPacket *cp) {
	char *cd = " " ;
#ifdef ESP8266
	return;
#else
	
	
	printf ( "%2s %24s: %4u %42s\n", cd, "Action", cp->action, actionLookup (cp->action) );
	printf ( "%2s %24s: %4u %42u\n", cd, "Control Index", cp->action_control, cp->action_control);
	printf ( "%2s %24s: %4u %42s\n", cd, "Control Type", cp->action_type, actionTypeLookup(cp->action_type));
	printf ( "%2s %24s: %4u %42u\n", cd, "Control Value", cp->action_value, cp->action_value);
	

#endif

}



void CP_ICACHE_FLASH_ATTR
chatPacket_print (chatPacket *cp, enum chatPacketDirection d) {

#ifdef ESP8266
	return;
#else

	uint32_t i, pl=0;
#ifdef IOS_APP
	uuid_string_t str;
#else
	char *str;
#endif
	char p;

	char *cd = " ";

	pl=cp->payloadLength;
	if ( d == NONE ) {
		printf ( "\n === ChatPacket ========================================== \n");
		cd = " ";
	} else if ( d == IN ) {
		printf ( "\n >>> ChatPacket RECEIVED >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
		cd = ">";
	} else if ( d == OUT ) {
		printf ( "\n <<< ChatPacket SENDING <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< \n");	
		cd = "<";
	}
	
	printf ( "%2s %24s %s\n", cd, "Field", "Value");
	printf ( "%2s %24s %s\n", cd, "---------", "---------");

	printf ( "%2s %24s: %42s\n", cd, "Command", cmdLookup (cp->cmd) );
	printf ( "%2s %24s: %42x\n", cd, "flags", cp->flags);
	printf ( "%2s %24s: %42u\n", cd, "serial", cp->serial);



	uuidToStr(&str, &cp->to.u0);
	printf ( "%2s %24s: %42s\n", cd, "to0", str);
#ifndef IOS_APP
	free(str);
#endif
	uuidToStr(&str, &cp->to.u1 );
	printf ( "%2s %24s: %42s\n", cd, "to1", str);
#ifndef IOS_APP
	free(str);
#endif
	uuidToStr(&str, &cp->from.u0);
	printf ( "%2s %24s: %42s\n", cd, "from0", str);
#ifndef IOS_APP
	free(str);
#endif
	uuidToStr(&str, &cp->from.u1);
	printf ( "%2s %24s: %42s\n", cd, "from0", str);
#ifndef IOS_APP
	free(str);
#endif

	
	printf ( "%2s %24s:%s", cd, "nonce", " " );

	util_print_bin2hex((unsigned char *)&cp->nonce, crypto_secretbox_NONCEBYTES);

	printf ( "%2s %24s: %42u\n", cd, "envelopeLength", cp->envelopeLength);

	printf ( "%2s %24s: %42x\n", cd, "envelope Padding Length", cp->envelopeRandomPaddingLength);
	printf ( "%2s %24s: ", cd, "envelope Padding");
	util_print_bin2hex((unsigned char *)&cp->envelopeRandomPaddingLength, 16);

	printf ( "%2s %24s: %42d\n", cd, "PayLoadLength", cp->payloadLength);

	printf ( "%2s %24s: %42x\n", cd, "payload Padding Length", cp->payloadRandomPaddingLength);
	printf ( "%2s %24s: ", cd, "payload Padding");
	util_print_bin2hex((unsigned char *)&cp->payloadRandomPadding, 16);
#ifndef IOS_APP
	printf ( "%2s %24s: ", cd, "payload" );
	for (i=0; i<cp->payloadLength; i++) {
		p = cp->payload[i];
		if ( p < 32 ) {
			printf (" ");
		} else {
			printf ("%c", p);
		}
		if ( (i > 0 ) && (i % 42) == 0 ) {
			printf ( "\n %24s: ", " " );		
		}
	}
	printf ("\n");
#endif
	printf ( "\n __________ / ChatPacket __________________________________________________ \n");
#endif
}

