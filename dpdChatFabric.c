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
#include <assert.h>
const char * stateLookup (enum chatPacketStates state) {

	switch (state) {
		case UNCONFIGURED:
		 return "UNCONFIGURED";
		break;
		case PUBLICKEY_SETUP:
		 return "PUBLICKEY_SETUP";
		break;
		case PAIRING_SETUP:
		 return "PAIRING_SETUP";
		break;
		case NONCE_SETUP:
		 return "NONCE_SETUP";
		break;
		case CONFIGURED:
		 return "CONFIGURED";
		break;
		case CONFIGURED_SYN:
		 return "CONFIGURED_SYN";
		break;
		case CONFIGURED_SYNACK:
		 return "CONFIGURED_SYNACK";
		break;
		case PAIRED:
		 return "PAIRED";
		break;
	}
	return "UNKNWON";
}



const char * cmdLookup (enum chatPacketCommands cmd) {
	switch (cmd) {	
	case INVAILD_CMD:
		return "INVAILD_CMD";
	break;
	case HELLO:
		return "HELLO";
	break;
	case PAIR_REQUEST:
		return "PAIR_REQUEST";
	break;
	case PAIR_REQUEST_ACK:
		return "PAIR_REQUEST_ACK";
	break;
	case HELLO_ACK:
		return "HELLO_ACK";
	break;
	case NONCE_REQUEST:
		return "NONCE_REQUEST";
	break;
	case NONCE_SEND:
		return "NONCE_SEND";
	break;
	case NONCE_RESET:
		return "NONCE_RESET";
	break;
	case PUBLICKEY_REQUEST:
		return "PUBLICKEY_REQUEST";
	break;
	case PUBLICKEY_SEND:
		return "PUBLICKEY_SEND";
	break;
	case PUBLICKEY_RESET:
		return "PUBLICKEY_RESET";
	break;
	case VERIFY_SYN:
		return "VERIFY_SYN";
	break;
	case VERIFY_SYNACK:
		return "VERIFY_SYNACK";
	break;
	case VERIFY_ACK:
		return "VERIFY_ACK";
	break;
	case APP_MESSAGE:
		return "APP_MESSAGE";
	break;
	case APP_REGISTER:
		return "APP_REGISTER";
	break;
	case APP_LIST:
		return "APP_LIST";
	break;
	case CONFIG_MESSAGE:
		return "CONFIG_MESSAGE";
	break;
	case CONFIG_DELIGATE:
		return "CONFIG_DELIGATE";
	break;
	case CONFIG_PAIR:
		return "CONFIG_PAIR";
	break;
	case SEND_REPLY_FALSE:
		return "SEND_REPLY_FALSE";
	break;
	case SEND_REPLY_TRUE:
		return "SEND_REPLY_TRUE";
	break;
	case PAIR_REQUEST_RESET:
		return "PAIR_REQUEST_RESET";
	break;
	case NONCE_ACK:
		return "NONCE_ACK";
	break; 
	case PUBLICKEY_ACK:
		return "PUBLICKEY_ACK";
	break;
	
	}
	return "UNKNOWN";


}

void print_bin2hex(unsigned char * x, int len) {
	int i;
	for (i=0; i<len; i++) {
		printf ( "%02x", x[i] );
		if ( (i>0) && ( (i+1)%4 == 0 ) ) { printf (" "); }
	}
	printf ("\n");

}

void 
chatFabric_hexprint ( unsigned char *str, uint32_t len ){
	int i;
	unsigned char p;
	printf ( "\n %4s: ", " " );		

	for (i=0; i<len; i++) {
		p = str[i];
		if ( p < 32 ) {
			printf (" ");
		} else {
			printf ("%c", p);
		}
		if ( (i > 0 ) && (i % 42) == 0 ) {
			printf ( "\n %4s: ", " " );		
		}
	}

	printf ("\n");

}

void 
chatFabric_hex2int_bytes (unsigned char *hex, uint32_t hexLength, unsigned char *dst, uint32_t dstLenght ) {
	char str[5] = { '0', 'x', '0', '0' , 0 };
	int i=0,x=0;
	uint32_t dst_int;
	//str[2] = 0;
//	printf ( "==> chatFabric_hex2int_bytes\n" );
	for ( i = 0; i<hexLength; i=i+2) {
		str[2] = hex[i];
		str[3] = hex[i+1];
		if ( x < dstLenght ) {
			sscanf (str, "%d", &dst_int );
			dst_int = strtoul(str, NULL, 16);
			dst[x] = (unsigned char)dst_int;
		}
		
//		printf ( " === %6s ?= %02x = %4d = ===========\n", str, dst[x], dst_int );
		
		x++;
	}

}

void
chatFabric_usage(char *p) {
	printf ("%s -config configfile\n", p);
	printf ("   -c --config              config file to use \n");
	printf ("   -d --db                  certificate database\n");
	printf ("   -i --ip                  ip address\n");

	return;
}

void
chatFabric_args(int argc, char**argv, chatFabricConfig *config) {
	int ch;

	static struct option longopts[] = {
		{	"config",
			required_argument,
			NULL,
			'c'
		},

		{	"db",
			required_argument,
			NULL,
			'd'
		},

		{	"ip",
			required_argument,
			NULL,
			'i'
		},

		/*  remember a zero line, else 
			getopt_long segfaults with unknown options */
	    {NULL, 
	    0, 
	    0, 
	    0}, 
			
	};


	while ((ch = getopt_long(argc, argv, "c:d:", longopts, NULL)) != -1) {
		switch (ch) {
			case 'c':
				printf ( "Arg --config : Value : %s \n", optarg );
				config->configfile = optarg;
			break;
			case 'd':
				printf ( "Arg --db : Value : %s \n", optarg );
				config->dbfile = optarg;
			break;
			case 'i':
				printf ( "Arg --ip : Value : %s \n", optarg );
				config->ip = optarg;
			break;

			case '?':
			case 'h':
			default:
				chatFabric_usage(argv[0]);
			break;
		}
	}
	argc -= optind;
	argv += optind;


}


void
chatFabric_configParse(chatFabricConfig *config) {

	struct ucl_parser *parser = NULL;
	ucl_object_t *root = NULL;
	const ucl_object_t *obj = NULL;
	uint32_t status;
	
	parser = ucl_parser_new(UCL_PARSER_KEY_LOWERCASE);
	
	if (parser == NULL) {
		perror("Could not allocate ucl parser");
		exit(1);
	}
	
	if (!ucl_parser_add_file(parser, config->configfile)) {
		if (errno != ENOENT) {
			perror("Parse error in file");
			//, config->configfile, ucl_parser_get_error(parser
			exit(EXIT_FAILURE);
			ucl_parser_free(parser);
		}
	}
	
	root = ucl_parser_get_object(parser);
	if (root == NULL || ucl_object_type(root) != UCL_OBJECT) {
		perror("Invalid configuration format.\n");	
		exit(EXIT_FAILURE);
	}

	obj = ucl_lookup_path(root, "uuid.id0");
	uuid_from_string(
		ucl_object_tostring(obj), 
		&(config->uuid0), 
		&status);
	
	obj = ucl_lookup_path(root, "uuid.id1");
	uuid_from_string(
		ucl_object_tostring(obj),  
		&(config->uuid1), 
		&status
	);

	obj = ucl_lookup_path(root, "keys.private");
	config->privatekey_str = (unsigned char *)ucl_object_tostring(obj);

	chatFabric_hex2int_bytes(
		config->privatekey_str,
		64,
		(unsigned char *)&config->privatekey,
		crypto_box_SECRETKEYBYTES
		);
	
	obj = ucl_lookup_path(root, "keys.public");
	config->publickey_str =	(unsigned char *)ucl_object_tostring(obj);

	chatFabric_hex2int_bytes(
		config->publickey_str,
		64,
		(unsigned char *)&config->publickey,
		crypto_box_PUBLICKEYBYTES
		);
		



	obj = ucl_lookup_path(root, "keys2.private");
	config->privatekey_str2 = (unsigned char *)ucl_object_tostring(obj);

	chatFabric_hex2int_bytes(
		config->privatekey_str2,
		64,
		(unsigned char *)&config->privatekey2,
		crypto_box_SECRETKEYBYTES
		);
	
	obj = ucl_lookup_path(root, "keys2.public");
	config->publickey_str2 =	(unsigned char *)ucl_object_tostring(obj);

	chatFabric_hex2int_bytes(
		config->publickey_str2,
		64,
		(unsigned char *)&config->publickey2,
		crypto_box_PUBLICKEYBYTES
		);
		

}

chatPacket*
chatPacket_init0 (void) {
	chatPacket * cp;
	unsigned char h, l, hp, lp; // high / low envelope and payload padding

	int i=0;
	unsigned int status=0;
	cp=calloc(1,sizeof(chatPacket));
	if  ( cp == 0 ) {
		return 0;
	}

	cp->cmd = 0;
	cp->flags = 0;
	//cp->nonce = 0;
	cp->envelopeLength = 0;

	cp->envelopeRandomPaddingLength = 0;
	for (i=0; i<16; i++) {
		cp->envelopeRandomPadding[i]=0;
		cp->payloadRandomPadding[i]=0;
	}

	uuid_create_nil(&(cp->to0), &status);
	uuid_create_nil(&(cp->to1), &status);
	uuid_create_nil(&(cp->from0), &status);
	uuid_create_nil(&(cp->from1), &status);

	cp->payloadLength =0;

	for (i=0; i<crypto_box_PUBLICKEYBYTES; i++) {
		cp->publickey[i] = 0;
	}

	cp->payloadRandomPaddingLength = 0;
	
	cp->payload = 0;
	
	h =  arc4random_uniform(15) + 1;
	l = 16 - h;
	cp->envelopeRandomPaddingLength = (h << 4) | l;
	arc4random_buf(&(cp->envelopeRandomPadding), 16);

	hp =  arc4random_uniform(15) + 1;
	lp = 16 - hp;
	cp->payloadRandomPaddingLength = (hp << 4) | lp;
	arc4random_buf(&(cp->payloadRandomPadding), 16);
	
	
	return cp;


}


chatPacket*
chatPacket_init (chatFabricConfig *config, chatFabricPairing *pair, enum chatPacketCommands cmd, unsigned char *payload, uint32_t len, uint32_t flags) {

	chatPacket * cp;
	uuid_tuple *to = &(pair->uuid);
	unsigned char h, l, hp, lp; // high / low envelope and payload padding

	cp=calloc(1,sizeof(chatPacket));
	if  ( cp == 0 ) {
		return 0;
	}

	cp->payload = calloc(len,sizeof(unsigned char) );
	if ( cp->payload == 0 ) {
		free(cp);
		return 0;	
	}


	h =  arc4random_uniform(15) + 1;
	l = 16 - h;
	cp->envelopeRandomPaddingLength = (h << 4) | l;
	arc4random_buf(&(cp->envelopeRandomPadding), 16);

	hp =  arc4random_uniform(15) + 1;
	lp = 16 - hp;
	cp->payloadRandomPaddingLength = (hp << 4) | lp;
	arc4random_buf(&(cp->payloadRandomPadding), 16);
		
	cp->to0 = to->u0;
	cp->to1 = to->u1;

	memcpy(&(cp->from0), &(config->uuid0), 16);
	memcpy(&(cp->from1), &(config->uuid1), 16);		

	cp->cmd = cmd;
	cp->flags = flags;
	
	cp->payloadLength = len;	
	memcpy(cp->payload, payload, len);

	// memcpy(cp->publickey, pair->publickey, crypto_box_PUBLICKEYBYTES);
	//cp->nonce = pair->nonce;
	// memcpy(  &(cp->nonce), &(pair->nonce), crypto_secretbox_NONCEBYTES );
	
		
	return cp;

}

void
chatPacket_delete (chatPacket* cp) {

	free(cp->payload);
	free(cp);

}


void
chatPacket_encode (chatPacket *cp, chatFabricConfig *config, chatFabricPairing *pair, msgbuffer *ob, int encrypted, enum chatPacketPacketTypes packetType) {
	uint32_t p_length =0, e_length=0, ob_length=0;
	unsigned long long p_length_encrpyted=0;
	uint32_t i;
	uint32_t ni;

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

/* 
***********************************
	Payload 
	    X bytes actual payload
	   16 bytes of random padding
	    1 byte length of random padding
	    4 bytes of tags (payload, random_high, random_low, random_length)	
*/
	h = 0;
	l = 0;

	h = cp->payloadRandomPaddingLength & 0xF0;
	h = h >> 4;	
	l = cp->payloadRandomPaddingLength  & 0x0F;	
	i = 0;
	


	p_length = 1+cp->payloadLength  + 2 + 1+16 + 1+4 + 1;
	
	
	
	if ( encrypted == _CHATPACKET_ENCRYPTED ) {
		// encryption overhead + tag+payload length
		payload=calloc(p_length,sizeof(unsigned char));
		
		
		p_length_encrpyted = p_length + crypto_secretbox_MACBYTES; 
		payload_encrypted=calloc(p_length_encrpyted,sizeof(unsigned char));	
		// Payload Length
		//tag
		c = cptag_payloadLength;
		memcpy(payload+i, &c, 1);
		++i;
		

		//data
		ni = htonl(cp->payloadLength);
		memcpy(payload+i, &ni, 4);
		i += 4;
		

//		p_length + p_length_encrpyted + 1+4 + 1
		ob_length+=p_length_encrpyted + 1+4 + 1;
		
		
	} else {
		payload=calloc(p_length,sizeof(unsigned char));
		ob_length+=p_length;
		
	}
	
	
// Payload Random Padding Length 
	// Tag		
	c = cptag_payloadRandomPaddingLength;
	memcpy(payload+i, &c, 1);
	++i;
	
	// data
	memcpy(payload+i, &(cp->payloadRandomPaddingLength), 1);
	++i;
	



// Payload Random Padding - High
	// Tag
	c = cptag_payloadRandomPaddingHigh;
	memcpy(payload+i, &c, 1);
	++i;
		
	// data
	memcpy(payload+i, &(cp->payloadRandomPadding), h);
	i += h;
		

// Payload
	// tag
	c = cptag_payload;
	memcpy(payload+i, &c, 1);
	++i;
		
	// data
	memcpy(payload+i, cp->payload, cp->payloadLength);
	i += cp->payloadLength;
		
	
// Payload Random Padding - Low
	// Tag
	c = cptag_payloadRandomPaddingLow;
	memcpy(payload+i, &c, 1);
	++i;
		
	// data
	memcpy(payload+i, &(cp->payloadRandomPadding[h]), l);
	i += l;
		


	if ( i != p_length ) {	
		printf ( " WARNING = PAYLOAD => index didn't match e_length! %u != %u\n", i, p_length );	
	}
	//assert( i <= p_length );
	
	if ( encrypted == _CHATPACKET_ENCRYPTED ) {

	printf ( "%2s %24s: ", "!!", "USING NONCE" );
	print_bin2hex((unsigned char *)&pair->nonce, crypto_secretbox_NONCEBYTES);
	printf ( "%2s %24s: ", "!!", "PUBLIC KEY" );
	print_bin2hex((unsigned char *)&pair->publickey, crypto_box_PUBLICKEYBYTES);

	printf ( "%2s %24s: ", "!!", "PUBLIC KEY1" );
	print_bin2hex((unsigned char *)&config->publickey, crypto_box_SECRETKEYBYTES);
	printf ( "%2s %24s: ", "!!", "PRIVATE KEY1" );
	print_bin2hex((unsigned char *)&config->privatekey, crypto_box_SECRETKEYBYTES);

	printf ( "%2s %24s: ", "!!", "PUBLIC KEY2" );
	print_bin2hex((unsigned char *)&config->publickey2, crypto_box_SECRETKEYBYTES);
	printf ( "%2s %24s: ", "!!", "PRIVATE KEY2" );
	print_bin2hex((unsigned char *)&config->privatekey2, crypto_box_SECRETKEYBYTES);

			// crypto_box_easy(ciphertext, MESSAGE, MESSAGE_LEN, nonce, bob_publickey, alice_secretkey);
			crypto_box_easy(
				payload_encrypted, 
				payload, p_length, 
				(const unsigned char *)&(pair->nonce),
				(unsigned char *)&(pair->publickey), 
				(unsigned char *)&(config->privatekey)
			);

	printf ( "%2s %24s: ", "!!", "ciphertext" );
	print_bin2hex((unsigned char *)payload_encrypted, p_length_encrpyted);

					if ( crypto_box_open_easy(
						payload,  
						payload_encrypted, 
						p_length + crypto_secretbox_MACBYTES, 
						(const unsigned char *)&(pair->nonce),
						(unsigned char *)&(config->publickey), 
						(unsigned char *)&(config->privatekey2)
					) == 0)
					{
						printf( " ===========> decryption success !!! ()\n");
					} else {
						printf( " ===========> decryption failed!\n");
					
					}
				
			free(payload);
			/*
			crypto_aead_chacha20poly1305_encrypt(payload_encrypted, &p_length_encrpyted,
												 (const unsigned char *)payload, 
												 (unsigned long long)p_length,
												 NULL, 0,
												 NULL, 
												 (const unsigned char *)&cp->payloadNonce, 
												 (const unsigned char *)&sharedkey_payload);
			*/

	}


}


/* 
***********************************
	Envelope 
***********************************
*/	

if (packetType == NONCE ) {

	// 2 + 1 + 16 + 1 + 8 + 1 + env len + crypto_aead_chacha20poly1305_ABYTES
	e_length =  2 + 1+16 + 1+crypto_secretbox_NONCEBYTES + 1;
		
	if ( encrypted == _CHATPACKET_ENCRYPTED ) {
		cp->envelopeLength = e_length + crypto_box_SEALBYTES;
		
	} else {
		cp->envelopeLength = e_length;
		
	}

	h = cp->envelopeRandomPaddingLength & 0xF0;
	h = h >> 4;
	l = cp->envelopeRandomPaddingLength & 0x0F;	
	i = 0;
		

	envelope = calloc(e_length,sizeof(unsigned char));

// envelope Random Padding Length 
	// Tag		
	c = cptag_envelopeRandomPaddingLength;
	memcpy(envelope+i, &c, 1);
	++i;
		
	// data
	memcpy(envelope+i, &(cp->envelopeRandomPaddingLength), 1);
	++i;
		

// envelope Random Padding - High
	// Tag
	c = cptag_envelopeRandomPaddingHigh;
	memcpy(envelope+i, &c, 1);
	++i;
		
	// data
	memcpy(envelope+i, &(cp->envelopeRandomPadding), h);
	i += h;
		

// envelopeNonce Nonce:
	//tag
	c = cptag_nonce;
	memcpy(envelope+i, &c, 1);
	++i;
		
	// data
	/*
	ni = htonl( (cp->nonce & 0xFFFFFFFF00000000) >> 32 );
	memcpy(envelope+i, &ni, 4);
	i += 4;
		
	ni = htonl( cp->nonce & 0x00000000FFFFFFFF );
	memcpy(envelope+i, &ni, 4);
	i += 4;
	*/
	memcpy(envelope+i, &(pair->mynonce), crypto_secretbox_NONCEBYTES);
	i += crypto_secretbox_NONCEBYTES;
	
	
		


// Payload Random Padding - Low
	// Tag
	c = cptag_envelopeRandomPaddingLow;
	memcpy(envelope+i, &c, 1);
	++i;
		
	// data
	memcpy(envelope+i, &(cp->envelopeRandomPadding[h]), l);
	i += l;
		
	
	if ( encrypted == _CHATPACKET_ENCRYPTED ) {
		envelope_encrypted =  calloc(cp->envelopeLength,sizeof(unsigned char));
		crypto_box_seal(envelope_encrypted, envelope, e_length, (unsigned char *)&(pair->publickey));
		ob_length+=cp->envelopeLength + 1 + 1+4 ; // + cptag_encryptedEnvelope + cptag_envelopeLength + cp->envelopeLength
		free(envelope);
		

		printf ("== envelope_encrypted (%5d) ========= ", cp->envelopeLength );
		print_bin2hex((unsigned char *)envelope_encrypted, cp->envelopeLength);
		
		printf ("== envelope_encrypted public key (%5d) ========= ", crypto_box_PUBLICKEYBYTES );
		print_bin2hex((unsigned char *)&(pair->publickey), crypto_box_PUBLICKEYBYTES);
		
	} else {
		ob_length+=cp->envelopeLength + 1+4;	
		
	}

}
		

	ob->length = ob_length;
	ob->msg = calloc(ob->length,sizeof(unsigned char));;
	i=0;
		

// Command
	//tag
	c = cptag_cmd;
	memcpy(ob->msg+i, &c, 1);
	++i;	
		
	//data
	ni = htonl(cp->cmd);
	memcpy(ob->msg+i, &ni, 4);
	i += 4;
		
	
// flags
	//tag
	c = cptag_flags;
	memcpy(ob->msg+i, &c, 1);
	++i;	
		
	//data
	ni = htonl(cp->flags);
	memcpy(ob->msg+i, &ni, 4);
	i += 4;
		

// UUID TO:
	//tag
	c = cptag_to0;
	memcpy(ob->msg+i, &c, 1);
	++i;	
		
	// data
	uuid_enc_be(ob->msg+i, &cp->to0);
	i += 16;
		
	
	//tag
	c = cptag_to1;
	memcpy(ob->msg+i, &c, 1);
	++i;	
		
	// data	
	uuid_enc_be(ob->msg+i, &cp->to1);
	i += 16;
		

// UUID FROM:
	
	//tag
	c = cptag_from0;
	memcpy(ob->msg+i, &c, 1);
	++i;	
		
	//data	
	uuid_enc_be(ob->msg+i, &cp->from0);
	i += 16;
		
	
	//tag
	c = cptag_from1;
	memcpy(ob->msg+i, &c, 1);
	++i;	
		
	//data		
	uuid_enc_be(ob->msg+i, &cp->from1);
	i += 16;
		

if (packetType == PUBLICKEY ) {
	//tag
	c = cptag_publickey;
	memcpy(ob->msg+i, &c, 1);
	++i;	
		
	// data
	memcpy(ob->msg+i, &(cp->publickey), crypto_box_PUBLICKEYBYTES);
	i += crypto_box_PUBLICKEYBYTES;
		
} 

if (packetType == NONCE ) {
	
	// flags
		//tag
		c = cptag_envelopeLength;
		memcpy(ob->msg+i, &c, 1);
		++i;	
		
		//data
		ni = htonl(cp->envelopeLength);
		memcpy(ob->msg+i, &ni, 4);
		i += 4;
		

	if ( encrypted == _CHATPACKET_ENCRYPTED ) {
		printf ("== envelope (%5d) ========= ", cp->envelopeLength );
		print_bin2hex((unsigned char *)envelope_encrypted, cp->envelopeLength);

		//tag
		c = cptag_encryptedEnvelope;
		memcpy(ob->msg+i, &c, 1);
		++i;	
		
	
		memcpy(ob->msg+i, envelope_encrypted, cp->envelopeLength);
		i += cp->envelopeLength;
		free(envelope_encrypted);
		
	} else {
		printf ("== envelope (%5d) ========= ", e_length );
		print_bin2hex((unsigned char *)envelope, e_length);

		memcpy(ob->msg+i, envelope, e_length);
		i += e_length;
		free(envelope);
		
	}

}

if (packetType == DATA ) {

// Payload Length
	//tag
	c = cptag_payloadLength;
	memcpy(ob->msg+i, &c, 1);
	++i;	
		
	//data
	if ( encrypted == _CHATPACKET_ENCRYPTED ) {
		ni = htonl(p_length_encrpyted);
	} else {
		ni = htonl(cp->payloadLength);
	}
	memcpy(ob->msg+i, &ni, 4);
	i += 4;
		

// copy payload into output buffer 

	if ( encrypted == _CHATPACKET_ENCRYPTED ) {
	// Tag
		c = cptag_encryptedPayload;
		memcpy(ob->msg+i, &c, 1);
		++i;
		
		memcpy(ob->msg+i, payload_encrypted, p_length_encrpyted);
		i += p_length_encrpyted;
		free(payload_encrypted);
		
	} else {
		memcpy(ob->msg+i, payload, p_length);
		i += p_length;
		free(payload);
		
	}
	
	
}

	if ( i != ob_length ) {	
		printf ( " WARNING = OB Length => index didn't match e_length! %u != %u\n", i, ob_length );	
	}

	printf ( " ==> Encoded Packet (%d): ",  ob_length ); 
	print_bin2hex((unsigned char *)ob->msg,ob_length);

		
}


// FIXME: should add size_t len, and assert ( b+x < b+len )
int chatPacket_decode (chatPacket *cp,  chatFabricPairing *pair, unsigned char *b, const int len, chatFabricConfig *config) {
	uint32_t ni=0, i=0, length = 0;
	unsigned char c=0, h=0, l=0, hp=0, lp = 0;
	unsigned char *decrypted=0;
	unsigned long long decrypted_len=0;
	int ret;
	const int len2 = len;
	printf ( " ==> Encoded Packet (%d): ", (int)len  ); 
	print_bin2hex((unsigned char *)b,(int)len);

	
	while (i<len2) {
		printf ( " == [decode] i = %d, %d, %d \n", i, len, len2 );
		memcpy(&c, b+i, 1);
		++i;
		
		printf (" ==== Processing Tag %d \n", c );
		switch (c){
			case cptag_cmd:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->cmd = ntohl(ni);
			break;
			case cptag_nonce:
			/*
				memcpy(&ni, b+i, 4);
				i+=4;
				ni = ntohl(ni);
				ni64 = ni;
				ni64 = ( ni64 << 32 );

				memcpy(&ni, b+i, 4);
				i+=4;
				ni = ntohl(ni);
				cp->nonce =  ni64 | ni;				
			*/
				memcpy(cp->nonce, b+i, crypto_secretbox_NONCEBYTES);
				i += crypto_secretbox_NONCEBYTES;
				
			break;
			
			case cptag_envelopeLength:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->envelopeLength = ntohl(ni);
//				printf ( " ==> Decoding envelope length : %5u ::", cp->envelopeLength ); 
//				print_bin2hex(b+i-5, 5);
			break;				
			case cptag_encryptedEnvelope:
					decrypted=calloc(cp->envelopeLength - crypto_box_SEALBYTES,sizeof(unsigned char));
					printf ("== envelope_encrypted (%5d) ========= ", cp->envelopeLength );
					print_bin2hex(b+i, cp->envelopeLength);

					printf ("== envelope_encrypted public key (%5d) ========= ", crypto_box_PUBLICKEYBYTES );
					print_bin2hex((unsigned char *)&(config->publickey), crypto_box_PUBLICKEYBYTES);

					printf ("== envelope_encrypted private key (%5d) ========= ", crypto_box_SECRETKEYBYTES );
					print_bin2hex((unsigned char *)&(config->privatekey), crypto_box_PUBLICKEYBYTES);

					ret = crypto_box_seal_open(
						decrypted, 
						b+i, cp->envelopeLength, 
						(unsigned char *)&(config->publickey), 
						(unsigned char *)&(config->privatekey));
					if ( ret == 0)
					{
						i+=cp->envelopeLength;
						if ( chatPacket_decode (cp, pair, decrypted, cp->envelopeLength - crypto_box_SEALBYTES, config) == 0 ) {
							free(decrypted);
							return 0;
						}
					} else {
						printf( " ===========> decryption failed! (%d)\n", ret);
						free(decrypted);
						return 0;
					}
					free(decrypted);
			break;
			case cptag_envelopeRandomPaddingLength:
				memcpy(&(cp->envelopeRandomPaddingLength), b+i, 1);
				i+=1;
				h = cp->envelopeRandomPaddingLength & 0xF0;				
				h = h >> 4;
				l = cp->envelopeRandomPaddingLength & 0x0F;				
			break;
			case cptag_envelopeRandomPaddingHigh:
				assert ( h != 0 ); // zero length is invalid padding.
				memcpy(&(cp->envelopeRandomPadding), b+i, h);
				i+=h;
			break;			
			case cptag_envelopeRandomPaddingLow:
				assert ( l != 0 ); // zero length is invalid padding.
				memcpy(&(cp->envelopeRandomPadding[h]), b+i, l);
				i+=l;
			break;
			case cptag_to0:
				uuid_dec_be(b+i, &cp->to0);
				i+=16;
			break;
			case cptag_to1:
				uuid_dec_be(b+i, &cp->to1);
				i+=16;
			break;
			case cptag_from0:
				uuid_dec_be(b+i, &cp->from0);
				i+=16;
			break;
			case cptag_from1:
				uuid_dec_be(b+i, &cp->from1);
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
					cp->payload=calloc(length,sizeof(unsigned char));
				}				
				cp->payloadLength = length;
				
			break;			
			case cptag_payloadRandomPaddingLength:
				memcpy(&(cp->payloadRandomPaddingLength), b+i, 1);
				i+=1;
				hp = cp->payloadRandomPaddingLength & 0xF0;
				hp = hp >> 4;
				lp = cp->payloadRandomPaddingLength & 0x0F;			
				printf ("== cptag_payloadRandomPaddingLength ( i=%d l=%d,%d bl=%d) ======\n", i, hp,lp, len );
			break;
			case cptag_payloadRandomPaddingHigh:
				printf ("== cptag_payloadRandomPaddingHigh ( i=%d hp=%d bl=%d) ======\n", i, hp, len );

				assert ( hp != 0 ); // zero length is invalid padding.
				memcpy(&(cp->payloadRandomPadding), b+i, hp);
				i+=hp;
			break;			
			
			case cptag_payloadRandomPaddingLow:
				printf ("== cptag_payloadRandomPaddingLow ( i=%d lp=%d bl=%d) ======\n", i, lp, len );
				assert ( lp != 0 ); // zero length is invalid padding.
				memcpy(&(cp->envelopeRandomPadding[hp]), b+i, lp);
				i+=lp;
				printf ("== cptag_payloadRandomPaddingLow ( i=%d lp=%d bl=%d) ======\n", i, lp, len );
			break;			
			case cptag_encryptedPayload:
				free(cp->payload);
				//cp->payload=calloc(cp->payloadLength - crypto_secretbox_MACBYTES,sizeof(unsigned char));
				
				decrypted=calloc(cp->payloadLength - crypto_secretbox_MACBYTES,sizeof(unsigned char));	
				
	printf ( "%2s %24s: ", "!!", "USING NONCE" );
	print_bin2hex((unsigned char *)&pair->mynonce, crypto_secretbox_NONCEBYTES);
	printf ( "%2s %24s: ", "!!", "PUBLIC KEY" );
	print_bin2hex((unsigned char *)&pair->publickey, crypto_box_PUBLICKEYBYTES);

	printf ( "%2s %24s: ", "!!", "PUBLIC KEY" );
	print_bin2hex((unsigned char *)&config->publickey, crypto_box_SECRETKEYBYTES);
	printf ( "%2s %24s: ", "!!", "PRIVATE KEY" );
	print_bin2hex((unsigned char *)&config->privatekey, crypto_box_SECRETKEYBYTES);

	printf ( "%2s %24s: ", "!!", "ciphertext" );
	print_bin2hex((unsigned char *)b+i, cp->payloadLength);


				if ( crypto_box_open_easy(
						decrypted, 
						(unsigned char *)b+i, cp->payloadLength, 
						(const unsigned char *)&(pair->mynonce),  
						(unsigned char *)&(pair->publickey), 
						(unsigned char *)&(config->privatekey)
				) == 0 )
				{
					i+=cp->payloadLength;

					printf( " ===========> decryption successful \n");
					printf ("== decrypted payload ( %d %d ) ======\n", cp->payloadLength, crypto_secretbox_MACBYTES );
					print_bin2hex(decrypted, cp->payloadLength - crypto_secretbox_MACBYTES);
					
					printf( " ===========> chatPacket_decode recursion  \n");
					if ( chatPacket_decode (cp, pair, decrypted, cp->payloadLength - crypto_secretbox_MACBYTES, config) == 0 ) {
						return 0;
					} else {
						printf( " ===========> chatPacket_decode recursions COMPLETE !  \n");					
					}
					
				} else {
					printf( " ===========> decryption failed!\n");
					return 0;
				}
				//cp->payloadLength -= crypto_secretbox_MACBYTES;

			break;
			case cptag_payload:
				printf ("== cptag_payload ( i=%d l=%d bl=%d) ======\n", i, cp->payloadLength, len );

				memcpy(cp->payload, b+i, cp->payloadLength);
				i+=cp->payloadLength;		
			break;
			case cptag_publickey:
				memcpy(&(cp->publickey), b+i, crypto_box_PUBLICKEYBYTES);
				i+=crypto_box_PUBLICKEYBYTES;
			break;
			default:
				print_bin2hex(b,len);
				printf ( " == BAD CHAT PACKET (%02x) =>> Last 6 bytes %02x %02x %02x %02x %02x %02x \n ", c, b[i-2],  b[i-1], b[i], b[i+1], b[i+2], b[i+3]);
				return 0;
				++i;
			break;
		}
	}
	
	return 1;

}

void chatPacket_print (chatPacket *cp, enum chatPacketDirection d) {

	uint32_t status,i;
	char *str;
	char p;

	char *cd = " ";
	
	printf ( "\n");
	if ( d == NONE ) {
		printf ( " === ChatPacket ========================================== \n");
		cd = " ";
	} else if ( d == IN ) {
		printf ( " >>> ChatPacket >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
		cd = ">";
	} else if ( d == OUT ) {
		printf ( " <<< ChatPacket <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< \n");	
		cd = "<";
	}
	printf ( "%2s %24s %s\n", cd, "Field", "Value");
	printf ( "%2s %24s %s\n", cd, "---------", "---------");

	printf ( "%2s %24s: %42s\n", cd, "Command", cmdLookup (cp->cmd) );
	printf ( "%2s %24s: %42x\n", cd, "flags", cp->flags);

	uuid_to_string(&cp->to0, &str, &status);
	printf ( "%2s %24s: %42s\n", cd, "to0", str);
	free(str);
	uuid_to_string(&cp->to1, &str, &status);
	printf ( "%2s %24s: %42s\n", cd, "to1", str);
	free(str);

	uuid_to_string(&cp->from0, &str, &status);
	printf ( "%2s %24s: %42s\n", cd, "from0", str);
	free(str);
	uuid_to_string(&cp->from1, &str, &status);
	printf ( "%2s %24s: %42s\n", cd, "from1", str);
	free(str);
	
	printf ( "%2s %24s: ", cd, "nonce" );
	print_bin2hex((unsigned char *)&cp->nonce, crypto_secretbox_NONCEBYTES);

	printf ( "%2s %24s: ", cd, "publicKey");
	print_bin2hex((unsigned char *)&cp->publickey, crypto_box_PUBLICKEYBYTES);


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
}


enum chatPacketCommands 
stateMachine (chatFabricConfig *config, chatFabricState *s, chatPacket *cp, chatFabricPairing *pair, chatPacket *reply)
{	
	chatFabricState previous_state;
	
	enum chatPacketCommands RETVAL;
	
	reply->to0 = cp->from0;
	reply->to1 = cp->from1;
	reply->from0 = config->uuid0;
	reply->from1= config->uuid1;
	previous_state.state =  s->state;
	previous_state.hasPublicKey = pair->hasPublicKey;
	
	RETVAL = SEND_REPLY_FALSE;

	printf ( " === [STATE] %-16s ==> %-16s \n", "COMMAND", cmdLookup(cp->cmd) );	


	switch ( cp->cmd ) {

		case PAIR_REQUEST_RESET:
			if ( s->state != PAIRED ) {
				s->state = UNCONFIGURED;
				reply->flags = s->state;
				reply->cmd = PAIR_REQUEST;
				RETVAL = SEND_REPLY_TRUE;
			}
		break;
		case HELLO:
			reply->flags = s->state;
			reply->cmd = HELLO_ACK;
			RETVAL = SEND_REPLY_TRUE;
		break;	

		case HELLO_ACK:
			reply->flags =  s->state;
			reply->cmd = HELLO_ACK;
			RETVAL = SEND_REPLY_FALSE;
		break;	
		case PAIR_REQUEST:
			if ( s->state == UNCONFIGURED ) {
				reply->flags = 0;
				reply->cmd = PAIR_REQUEST_ACK;
				s->state = PUBLICKEY_SETUP;
				pair->uuid.u0 = cp->from0;
				pair->uuid.u1 = cp->from1;
				RETVAL = SEND_REPLY_TRUE;	
			}
		break;
		case PAIR_REQUEST_ACK:
			if ( s->state == UNCONFIGURED ) {
				reply->flags = 0;
				reply->cmd = PUBLICKEY_REQUEST;
				s->state = PUBLICKEY_SETUP;
				pair->uuid.u0 = cp->from0;
				pair->uuid.u1 = cp->from1;
				RETVAL = SEND_REPLY_TRUE;
			}
		break;
		case NONCE_REQUEST:
			if  ( s->state == NONCE_SETUP ) {
				if ( pair->hasPublicKey == 1 ) {
					reply->flags = pair->hasNonce;
//					reply->nonce = pair->mynonce;
					memcpy(&(reply->nonce), &(pair->mynonce), crypto_secretbox_NONCEBYTES );
					reply->cmd = NONCE_SEND;
					s->state = NONCE_SETUP;
					RETVAL = SEND_REPLY_TRUE;
				} else {
					reply->flags = 0;
					s->state = UNCONFIGURED;
					reply->cmd = PAIR_REQUEST_RESET;
					RETVAL = SEND_REPLY_TRUE;
				}
			}
		break;
		case NONCE_ACK:
			if  ( s->state == NONCE_SETUP ) {
					s->state = PAIRED;
					RETVAL = SEND_REPLY_FALSE;
			}		
		break;
		case NONCE_SEND:
			if  ( s->state == NONCE_SETUP ) {
//				pair->nonce = cp->nonce;
				memcpy( &(pair->nonce), &(cp->nonce), crypto_secretbox_NONCEBYTES );
				
				pair->hasNonce = 1;
				reply->flags = 0;
				if ( cp->flags == 1 ) {
					reply->cmd = NONCE_ACK;
					s->state = PAIRED;				
				} else {
					reply->flags = pair->hasNonce;
					reply->cmd = NONCE_SEND;
//					reply->nonce = pair->mynonce;
					memcpy(&(reply->nonce), &(pair->mynonce), crypto_secretbox_NONCEBYTES );
					s->state = NONCE_SETUP;
				}
				RETVAL = SEND_REPLY_TRUE;				
			}			
		break;
		case NONCE_RESET:
			RETVAL = SEND_REPLY_FALSE;
		break;
		case PUBLICKEY_ACK:
			if  ( s->state == PUBLICKEY_SETUP ) {
				s->state = NONCE_SETUP;
				reply->flags = s->state;
				reply->cmd = NONCE_REQUEST;
				RETVAL = SEND_REPLY_TRUE;				
			}
		case PUBLICKEY_REQUEST:
			// FIXME: check UUIDs, Request PublicKey
			if  ( s->state == PUBLICKEY_SETUP  ) {
				reply->flags = pair->hasPublicKey;
				reply->cmd = PUBLICKEY_SEND;
				memcpy( &(reply->publickey), &(config->publickey), crypto_box_PUBLICKEYBYTES );
				s->state = PUBLICKEY_SETUP;
				RETVAL = SEND_REPLY_TRUE;	
			}
		break;
		case PUBLICKEY_SEND:
			if  ( s->state == PUBLICKEY_SETUP ) {
				memcpy( &(pair->publickey), &(cp->publickey), crypto_box_PUBLICKEYBYTES );
				pair->hasPublicKey = 1;
				// flags contains hasPublicKey? of remote 
				if ( cp->flags == 1 ) {
					reply->flags = pair->hasPublicKey;
					reply->cmd = PUBLICKEY_ACK;
					s->state = NONCE_SETUP;
				} else {
					reply->flags = pair->hasPublicKey;
					reply->cmd = PUBLICKEY_SEND;
					memcpy( &(reply->publickey), &(config->publickey), crypto_box_PUBLICKEYBYTES );
				}
				RETVAL = SEND_REPLY_TRUE;
			}	
		break;
		
		case PUBLICKEY_RESET:
				RETVAL = SEND_REPLY_FALSE;
		break;
		case VERIFY_SYN:
			if  ( ( s->state == CONFIGURED  )  || ( s->state == CONFIGURED_SYN  ) || ( s->state == CONFIGURED_SYNACK  ) )
			{
				reply->flags = 0;
				reply->cmd = VERIFY_SYNACK;
				s->state = CONFIGURED_SYN;
				RETVAL = SEND_REPLY_TRUE;	
			}		
		break;
		case VERIFY_SYNACK:
			if  ( ( s->state == CONFIGURED  )  || ( s->state == CONFIGURED_SYN  ) || ( s->state == CONFIGURED_SYNACK  ) )
			{
				reply->flags = 0;
				reply->cmd = VERIFY_ACK;
				s->state = CONFIGURED_SYNACK;
				RETVAL = SEND_REPLY_TRUE;	
			}
		break;
		case VERIFY_ACK:
			if  ( ( s->state == CONFIGURED  )  || ( s->state == CONFIGURED_SYN  ) || ( s->state == CONFIGURED_SYNACK  ) )
			{
				s->state = PAIRED;
				reply->flags = 0;
				reply->cmd = VERIFY_ACK;
				s->state = PAIRED;
				RETVAL = SEND_REPLY_TRUE;	
			}
		break;
				
		case APP_MESSAGE:
			reply->flags =  s->state;
			reply->cmd = HELLO_ACK;
			RETVAL = SEND_REPLY_TRUE;
		break;
		case APP_REGISTER:
				RETVAL = SEND_REPLY_FALSE;
		break;
		case APP_LIST:
				RETVAL = SEND_REPLY_FALSE;
		break;
		case CONFIG_MESSAGE:
				RETVAL = SEND_REPLY_FALSE;
		break;
		case CONFIG_DELIGATE:
				RETVAL = SEND_REPLY_FALSE;
		break;
		case CONFIG_PAIR:
				RETVAL = SEND_REPLY_FALSE;
		break;
		case SEND_REPLY_FALSE:
			RETVAL = SEND_REPLY_FALSE;
		break;
		case SEND_REPLY_TRUE:
			RETVAL = SEND_REPLY_FALSE;
		break;		
		default:
			reply->flags = 0;
			reply->cmd = INVAILD_CMD;
			RETVAL = SEND_REPLY_TRUE;
		break;

	}

	
	if ( previous_state.state == s->state ) {	
		printf ( " === [STATE] %-16s ==> %-16s %-36s \n", stateLookup(previous_state.state), stateLookup(s->state), "STATE WAS UNCHANGED" );	
	} else {
		printf ( " === [STATE] %-16s ==> %-16s %-36s \n", stateLookup(previous_state.state), stateLookup(s->state), "!!! STATE CHANGED !!" );	
	}
	
	if ( previous_state.hasPublicKey != pair->hasPublicKey ) {
		printf ( " === [STATE] %-16s ==> ", "PUBLICKEY_SET" );	
	
		print_bin2hex((unsigned char *)&(pair->publickey), crypto_box_PUBLICKEYBYTES);
	}

	
	return RETVAL;
}
