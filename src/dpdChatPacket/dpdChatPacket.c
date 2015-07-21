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

#ifdef ESP8266
void inline free(void *x) { return; }
#include "esp8266.h"
#endif

// #include <assert.h>
const char * 
ICACHE_FLASH_ATTR 
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
		case cptag_cmd:
			return "cmd";
		break;
	}
}


const char * 
ICACHE_FLASH_ATTR 
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
ICACHE_FLASH_ATTR
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
	
	}
	return "UNKNOWN";


}

void ICACHE_FLASH_ATTR
chatPacket_tagDataEncoder( enum chatPacketTagData type, unsigned char *b, uint32_t *i, unsigned char tag,  uint32_t value, unsigned char*s, uint32_t len, uuid_t *uuid)
{

	uint32_t x = *i;
	uint32_t ni = 0;
	
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
		uuid_enc_be(b+x, uuid);
		x += 16;
	}

	*i=x;
}



chatPacket*
ICACHE_FLASH_ATTR
chatPacket_init0 (void) {
	chatPacket * cp = NULL;
	unsigned char h, l, hp, lp; // high / low envelope and payload padding

	int i=0;
	unsigned int status=0;
#ifndef ESP8266	
	cp=(chatPacket *)calloc(1,sizeof(chatPacket));
	if  ( cp == 0 ) {
		return 0;
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
	

	cp->envelopeRandomPaddingLength = 0;
	for (i=0; i<16; i++) {
		cp->envelopeRandomPadding[i]=0;
		cp->payloadRandomPadding[i]=0;
	}

	uuid_create_nil(&(cp->to.u0), &status);
	uuid_create_nil(&(cp->to.u1), &status);
	uuid_create_nil(&(cp->from.u0), &status);
	uuid_create_nil(&(cp->from.u1), &status);

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
		
	return cp;


}


chatPacket*
ICACHE_FLASH_ATTR
chatPacket_init (chatFabricConfig *config, chatFabricPairing *pair, enum chatPacketCommands cmd, unsigned char *payload, uint32_t len, uint32_t flags) {

	chatPacket * cp;
	uuid_tuple *to = &(pair->uuid);
	unsigned char h, l, hp, lp; // high / low envelope and payload padding

#ifndef ESP8266	
	cp=(chatPacket *)calloc(1,sizeof(chatPacket));
	if  ( cp == 0 ) {
		return 0;
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


	h =  arc4random_uniform(15) + 1;
	l = 16 - h;
	cp->envelopeRandomPaddingLength = (h << 4) | l;
	arc4random_buf((unsigned char *)&(cp->envelopeRandomPadding), 16);

	hp =  arc4random_uniform(15) + 1;
	lp = 16 - hp;
	cp->payloadRandomPaddingLength = (hp << 4) | lp;
	arc4random_buf((unsigned char *)&(cp->payloadRandomPadding), 16);
		
	cp->to.u0 = to->u0;
	cp->to.u1 = to->u1;

	memcpy(&(cp->from.u0), &(config->uuid.u0), 16);
	memcpy(&(cp->from.u1), &(config->uuid.u1), 16);		

	cp->cmd = cmd;
	cp->flags = flags;
	cp->wasEncrypted = -1;
	cp->payloadLength = len;	
	memcpy(cp->payload, payload, len);
		
	return cp;

}

void
ICACHE_FLASH_ATTR
chatPacket_delete (chatPacket* cp) {
#ifndef ESP8266
	free(cp->payload);
	free(cp);
#else
	cpStatus[cp->cpindex] = -1;
#endif
}


void
ICACHE_FLASH_ATTR
chatPacket_encode (chatPacket *cp, chatFabricConfig *config, chatFabricPairing *pair, msgbuffer *ob, int encrypted, enum chatPacketPacketTypes packetType) {
	uint32_t p_length =0, e_length=0, ob_length=0, encrypted_envolopeLength=0;
	unsigned long long p_length_encrpyted=0;
	uint32_t i;
	uint32_t ni, ni2;

	unsigned char c,h,l;
		
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
	
		if ( encrypted == _CHATPACKET_ENCRYPTED ) {
			payload=(unsigned char*)calloc(p_length,sizeof(unsigned char));		
		
			p_length_encrpyted = p_length + crypto_secretbox_MACBYTES; 
			payload_encrypted=(unsigned char*)calloc(p_length_encrpyted,sizeof(unsigned char));	
			
			chatPacket_tagDataEncoder ( CP_INT32, payload, &i, cptag_payloadLength, cp->payloadLength, NULL, 0, NULL);			
			
			ob_length+=p_length_encrpyted + 1+4 + 1;
				
		} else {
			payload=(unsigned char*)calloc(p_length,sizeof(unsigned char));
			ob_length+=p_length;		
		}
		

		chatPacket_tagDataEncoder ( CP_DATA8, payload, &i, cptag_payloadRandomPaddingLength, 0, &cp->payloadRandomPaddingLength, 1, NULL);
		chatPacket_tagDataEncoder ( CP_DATA8, payload, &i, cptag_payloadRandomPaddingHigh, 0, (unsigned char *)&cp->payloadRandomPadding, h, NULL);
		chatPacket_tagDataEncoder ( CP_DATA8, payload, &i, cptag_payload, 0, cp->payload, cp->payloadLength, NULL);
		chatPacket_tagDataEncoder ( CP_DATA8, payload, &i, cptag_payloadRandomPaddingLow, 0, (unsigned char *)&(cp->payloadRandomPadding[h]), l, NULL);

		if ( i != p_length ) {	
			printf ( " WARNING = PAYLOAD => index didn't match e_length! %u != %u\n", i, p_length );	
		}
	
		if ( encrypted == _CHATPACKET_ENCRYPTED ) {

			#ifdef HAVE_SODIUM
			crypto_box_easy(
				payload_encrypted, 
				payload, p_length, 
				(const unsigned char *)&(pair->nonce),
				(unsigned char *)&(pair->publickey), 
				(unsigned char *)&(config->privatekey)
			);
			#endif
			
			#ifdef HAVE_LOCAL_CRYPTO
			memcpy ( payload_encrypted, payload, p_length );
			s20_crypt((uint8_t*)&pair->sharedkey, S20_KEYLEN_256, pair->nonce, 0, payload_encrypted, p_length);
			poly1305_auth(payload_encrypted+p_length, payload_encrypted, p_length, (unsigned char *)&pair->sharedkey);
			
			if (config->debug) {
				printf ( "   %24s: ", "MAC Calculated" );
				print_bin2hex((unsigned char *)payload_encrypted+p_length, crypto_secretbox_MACBYTES);
				printf ( "   %24s (%8d): ", "encrypted payload",  p_length_encrpyted);
				print_bin2hex((unsigned char *)payload_encrypted, p_length_encrpyted);
				printf ( "   %24s (%8d): ", "plaintext payload",  p_length);
				print_bin2hex((unsigned char *)payload, p_length);


				printf ( "   %24s: ", "nonce" );
				print_bin2hex((unsigned char *)pair->nonce, crypto_secretbox_NONCEBYTES);
				printf ( "   %24s: ", "shared key" );
				print_bin2hex((unsigned char *)&pair->sharedkey, crypto_box_PUBLICKEYBYTES);

			}
			
			#endif

			
			
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
			print_bin2hex (envelope, e_length );		
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
				print_bin2hex((unsigned char *)envelope_encrypted+e_length, crypto_secretbox_MACBYTES);
				printf ( "   %24s (%8d): ", "encrypted envelope",  e_length);
				print_bin2hex((unsigned char *)envelope_encrypted, e_length);
			}
			
			#endif			
			if (config->debug){ 
				CHATFABRIC_DEBUG(config->debug, " ======> encrypted Envelope : " );
				print_bin2hex (envelope_encrypted, encrypted_envolopeLength );		
			}

			cp->envelopeLength = encrypted_envolopeLength;
			ob_length+=encrypted_envolopeLength + 1 + 1+4 ; // + cptag_encryptedEnvelope + cptag_envelopeLength + cp->envelopeLength
			free(envelope);
		} else {
			ob_length+=encrypted_envolopeLength + 1+4;			
		}
	}
	
	/*
	if (cp->payloadLength == 0 )
	{
		ob_length+=5;
	}
	*/
	ob->length = ob_length;
	ob->msg = (unsigned char*)calloc(ob->length,sizeof(unsigned char));;
	i=0;
		
	chatPacket_tagDataEncoder ( CP_INT32, ob->msg, &i, cptag_cmd, cp->cmd, NULL, 0, NULL);
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
		print_bin2hex (ob->msg, ob->length );		
	}
		
}


int 
ICACHE_FLASH_ATTR
chatPacket_decode (chatPacket *cp,  chatFabricPairing *pair, unsigned char *b, const int len, chatFabricConfig *config) {
	uint32_t ni=0, i=0, length = 0;
	unsigned char c=0, h=0, l=0, hp=0, lp = 0;
	unsigned char *decrypted=0;
	int ret;
	unsigned char * mac;		

	if (config->debug) {
		CHATFABRIC_DEBUG(config->debug, " decoding Packet : " );
		print_bin2hex (b, len);		
	}
	
//	const int len2 = len;

	while (i<len) {
		memcpy(&c, b+i, 1);
		++i;
		
		switch (c){
			case cptag_cmd:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->cmd = ntohl(ni);
					
			break;
			case cptag_nonce:
				memcpy(cp->nonce, b+i, crypto_secretbox_NONCEBYTES);
				i += crypto_secretbox_NONCEBYTES;				
			break;
			
			case cptag_envelopeLength:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->envelopeLength = ntohl(ni);
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
					free(mac);	
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
						if ( chatPacket_decode (cp, pair, decrypted, cp->envelopeLength - crypto_box_SEALBYTES, config) == 0 ) {
							free(decrypted);
							return 0;
						}
					} else {
						free(decrypted);
						CHATFABRIC_DEBUG(config->debug,  " ===> Decryption (envelope) Failed \n" );
						CHATFABRIC_DEBUG_FMT(config->debug,  
							"[DEBUG][%s:%s:%d]  i = %4d,   el = %4d \n",
							__FILE__, __FUNCTION__, __LINE__,  i, cp->envelopeLength);
						if (config->debug) {
							printf ( "   %24s (%8d): ", "MAC Calculated" , crypto_secretbox_MACBYTES);
							print_bin2hex((unsigned char *)mac, crypto_secretbox_MACBYTES);
							printf ( "   %24s (%8d): ", "MAC Stream",  cp->envelopeLength);
							print_bin2hex((unsigned char *)b+i+cp->envelopeLength-crypto_secretbox_MACBYTES, crypto_secretbox_MACBYTES);
							printf ( "   %24s (%8d): ", "encrypted envelope",  cp->envelopeLength);
							print_bin2hex((unsigned char *) b+i, cp->envelopeLength);
						}
						return -1;
					}
			break;
			case cptag_envelopeRandomPaddingLength:
				memcpy(&(cp->envelopeRandomPaddingLength), b+i, 1);
				i+=1;
				h = cp->envelopeRandomPaddingLength & 0xF0;				
				h = h >> 4;
				l = cp->envelopeRandomPaddingLength & 0x0F;				
			break;
			case cptag_envelopeRandomPaddingHigh:
				if ( h == 0 ) {
					// zero length is invalid padding.
					CHATFABRIC_DEBUG(config->debug, " ===> cptag_envelopeRandomPaddingHigh == 0 \n");
					return -1;
				}
				memcpy(&(cp->envelopeRandomPadding), b+i, h);
				i+=h;
			break;			
			case cptag_envelopeRandomPaddingLow:
				if ( l == 0 ) {
					// zero length is invalid padding.
					CHATFABRIC_DEBUG(config->debug, " ===> cptag_envelopeRandomPaddingLow == 0 \n");
					return -1;
				}
				memcpy(&(cp->envelopeRandomPadding[h]), b+i, l);
				i+=l;
			break;
			case cptag_to0:
				uuid_dec_be(b+i, &cp->to.u0);
//				print_bin2hex((unsigned char *)b+i, 16);
				i+=16;
			break;
			case cptag_to1:
				uuid_dec_be(b+i, &cp->to.u1);
//				print_bin2hex((unsigned char *)b+i, 16);
				i+=16;
			break;
			case cptag_from0:
				uuid_dec_be(b+i, &cp->from.u0);
//				print_bin2hex((unsigned char *)b+i, 16);
				i+=16;
			break;
			case cptag_from1:
				uuid_dec_be(b+i, &cp->from.u1);
//				print_bin2hex((unsigned char *)b+i, 16);
				i+=16;
			break;
			case cptag_flags:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->flags = ntohl(ni);			
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
				
			break;			
			case cptag_payloadRandomPaddingLength:
				memcpy(&(cp->payloadRandomPaddingLength), b+i, 1);
				i+=1;
				hp = cp->payloadRandomPaddingLength & 0xF0;
				hp = hp >> 4;
				lp = cp->payloadRandomPaddingLength & 0x0F;			
			break;
			case cptag_payloadRandomPaddingHigh:
				if ( hp == 0 ) {
					// zero length is invalid padding.
					CHATFABRIC_DEBUG(config->debug, " ===> cptag_payloadRandomPaddingHigh == 0 \n");
					return -1;
				}
				memcpy(&(cp->payloadRandomPadding), b+i, hp);
				i+=hp;
			break;			
			
			case cptag_payloadRandomPaddingLow:
				if ( lp == 0 ) {
					// zero length is invalid padding.
					CHATFABRIC_DEBUG(config->debug, " ===> cptag_payloadRandomPaddingLow == 0 \n");
					return -1;
				}
				memcpy(&(cp->envelopeRandomPadding[hp]), b+i, lp);
				i+=lp;
			break;			
			case cptag_encryptedPayload:
				cp->wasEncrypted = cp->wasEncrypted | 0x01;			
				//free(cp->payload);				
				
				#ifdef HAVE_LOCAL_CRYPTO
				mac=(unsigned char*)calloc(crypto_secretbox_MACBYTES,sizeof(unsigned char) );
				
				poly1305_auth(mac,  b+i,  cp->payloadLength - crypto_secretbox_MACBYTES, (unsigned char *)&pair->sharedkey);
				ret = poly1305_verify(mac, b+i+cp->payloadLength-crypto_secretbox_MACBYTES);
				free(mac);
//				ret =1 ;									
					CHATFABRIC_DEBUG(config->debug, " ===> Decrypting  (payload) \n");
					if (config->debug) {
						printf ( "   %24s: ", "nonce" );
						print_bin2hex((unsigned char *)pair->mynonce, crypto_secretbox_NONCEBYTES);
						printf ( "   %24s: ", "shared key" );
						print_bin2hex((unsigned char *)&pair->sharedkey, crypto_box_PUBLICKEYBYTES);
					}
									
				if ( ret == 1 ) {					
					decrypted=(unsigned char*)calloc(cp->payloadLength - crypto_secretbox_MACBYTES,sizeof(unsigned char));
					memcpy ( decrypted, b+i, cp->payloadLength - crypto_secretbox_MACBYTES );
					s20_crypt((uint8_t*)&pair->sharedkey, S20_KEYLEN_256, pair->mynonce, 0, decrypted, cp->payloadLength - crypto_secretbox_MACBYTES);
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
						(const unsigned char *)&(pair->mynonce),  
						(unsigned char *)&(pair->publickey), 
						(unsigned char *)&(config->privatekey) );
				#endif
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
					CHATFABRIC_DEBUG(config->debug, " ===> Decryption (payload) Failed \n");
					if (config->debug) {
						printf ( "   %24s: ", "MAC Calculated" );
						print_bin2hex((unsigned char *)mac, crypto_secretbox_MACBYTES);
						printf ( "   %24s: ", "MAC Stream" );
						print_bin2hex((unsigned char *)b+i+cp->payloadLength, crypto_secretbox_MACBYTES);
					}
					
					return -1;
				}

			break;
			case cptag_payload:
				memcpy(cp->payload, b+i, cp->payloadLength);
				i+=cp->payloadLength;		
			break;
			case cptag_publickey:
				memcpy(&(cp->publickey), b+i, crypto_box_PUBLICKEYBYTES);
				i+=crypto_box_PUBLICKEYBYTES;
			break;
			case cptag_envelope:
			break;
			default:
//				printf ( " == BAD CHAT PACKET (%02x) =>> Last 6 bytes %02x %02x %02x %02x %02x %02x \n ", c, b[i-2],  b[i-1], b[i], b[i+1], b[i+2], b[i+3]);
				CHATFABRIC_DEBUG_FMT(config->debug,  
					"[DEBUG][%s:%s:%d]  BAD CHAT PACKET (%02x) =>> \n",
					__FILE__, __FUNCTION__, __LINE__,  c );
				//assert(0);
				return -1;
				++i;
			break;
		}
	}
	
	return 0;

}

void ICACHE_FLASH_ATTR
chatPacket_print (chatPacket *cp, enum chatPacketDirection d) {

#ifdef ESP8266
	return;
#else

	uint32_t status,i, pl=0;
	char *str;
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

	uuid_to_string(&cp->to.u0, &str, &status);
	printf ( "%2s %24s: %42s\n", cd, "to0", str);
//	free(str);
	uuid_to_string(&cp->to.u1, &str, &status);
	printf ( "%2s %24s: %42s\n", cd, "to1", str);
//	free(str);
	uuid_to_string(&cp->from.u0, &str, &status);
	printf ( "%2s %24s: %42s\n", cd, "from0", str);
//	free(str);
	uuid_to_string(&cp->from.u1, &str, &status);
	printf ( "%2s %24s: %42s\n", cd, "from0", str);
//	free(str);

	
	printf ( "%2s %24s:%s", cd, "nonce", " " );

	print_bin2hex((unsigned char *)&cp->nonce, crypto_secretbox_NONCEBYTES);

	printf ( "%2s %24s: %42u\n", cd, "envelopeLength", cp->envelopeLength);

	printf ( "%2s %24s: %42x\n", cd, "envelope Padding Length", cp->envelopeRandomPaddingLength);
	printf ( "%2s %24s: ", cd, "envelope Padding");
	print_bin2hex((unsigned char *)&cp->envelopeRandomPaddingLength, 16);

	printf ( "%2s %24s: %42d\n", cd, "PayLoadLength", cp->payloadLength);

	printf ( "%2s %24s: %42x\n", cd, "payload Padding Length", cp->payloadRandomPaddingLength);
	printf ( "%2s %24s: ", cd, "payload Padding");
	print_bin2hex((unsigned char *)&cp->payloadRandomPadding, 16);
	
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
	printf ( "\n __________ / ChatPacket __________________________________________________ \n");
#endif
}

