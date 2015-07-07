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


void print_bin2hex(unsigned char * x, int len) {
	int i;
	for (i=0; i<len; i++) {
		printf ( "%02x", x[i] );
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
	printf ( "==> chatFabric_hex2int_bytes\n" );
	for ( i = 0; i<hexLength; i=i+2) {
		str[2] = hex[i];
		str[3] = hex[i+1];
		if ( x < dstLenght ) {
			sscanf (str, "%d", &dst_int );
			dst_int = strtoul(str, NULL, 16);
			dst[x] = (unsigned char)dst_int;
		}
		
		printf ( " === %6s ?= %02x = %4d = ===========\n", str, dst[x], dst_int );
		
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

	obj = ucl_lookup_path(root, "payloadkeys.private");
	config->payloadkeys_private_str = (unsigned char *)ucl_object_tostring(obj);

	chatFabric_hex2int_bytes(
		config->payloadkeys_private_str,
		64,
		&config->payloadkeys_private,
		sizeof(config->payloadkeys_private)
		);
	
	obj = ucl_lookup_path(root, "payloadkeys.public");
	config->payloadkeys_public_str =	(unsigned char *)ucl_object_tostring(obj);

	chatFabric_hex2int_bytes(
		config->payloadkeys_public_str,
		64,
		&config->payloadkeys_public,
		sizeof(config->payloadkeys_public)
		);


	obj = ucl_lookup_path(root, "envelopekeys.private");
	config->envelopekeys_private_str = (unsigned char *)ucl_object_tostring(obj);

	chatFabric_hex2int_bytes(
		config->envelopekeys_private_str,
		64,
		&config->envelopekeys_private,
		sizeof(config->envelopekeys_private)
		);

	
	obj = ucl_lookup_path(root, "envelopekeys.public");
	config->envelopekeys_public_str =	(unsigned char *)ucl_object_tostring(obj);


	chatFabric_hex2int_bytes(
		config->envelopekeys_public_str,
		64,
		&config->envelopekeys_public,
		sizeof(config->envelopekeys_public)
		);


	obj = ucl_lookup_path(root, "peerkeys.payloadpublic");
	config->peerkeys_payload_public_str =	(unsigned char *)ucl_object_tostring(obj);

	chatFabric_hex2int_bytes(
		config->peerkeys_payload_public_str,
		64,
		&config->peerkeys_payload_public,
		sizeof(config->peerkeys_payload_public)
		);


	obj = ucl_lookup_path(root, "peerkeys.envelopepublic");
	config->peerkeys_envelope_public_str =	(unsigned char *)ucl_object_tostring(obj);

	chatFabric_hex2int_bytes(
		config->peerkeys_envelope_public_str,
		64,
		&config->peerkeys_envelope_public,
		sizeof(config->peerkeys_envelope_public)
		);

	arc4random_buf(&(config->envelopeNonce), 8);
	arc4random_buf(&(config->payloadNonce), 8);

	config->msgid = arc4random_uniform(2^32 - 1);

}


chatPacket*
chatPacket_init (chatFabricConfig *config, uuid_tuple *to, unsigned char *payload, uint32_t len, uint32_t flags) {

	uint32_t status;
	chatPacket * cp;
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
	
	uuid_from_string(_UUID0, &(cp->payloadKey), &status);
	uuid_from_string(_UUID0, &(cp->envelopeKey), &status);
	
	cp->to0 = to->u0;
	cp->to1 = to->u1;

	memcpy(&(cp->from0), &(config->uuid0), 16);
	memcpy(&(cp->from1), &(config->uuid1), 16);		

	cp->flags = flags;
	cp->msgid = config->msgid;
	++config->msgid;
	
	cp->payloadLength = len;	
	memcpy(cp->payload, payload, len);
	
	cp->envelopeNonce = config->envelopeNonce;
	++config->envelopeNonce;

	cp->payloadNonce = config->payloadNonce;
	++config->payloadNonce;
	
	return cp;

}

void
chatPacket_delete (chatPacket* cp) {

	free(cp->payload);
	free(cp);

}


void
chatPacket_encode (chatPacket *cp, chatFabricConfig *config, msgbuffer *ob, int encrypted) {
	uint32_t p_length =0, e_length=0;
	unsigned long long p_length_encrpyted=0;
	uint32_t i;
	uint32_t ni;

	unsigned char c,h,l;
	
	unsigned char *envelope;
	unsigned char *payload;
	unsigned char *payload_encrypted;
	unsigned char sharedkey_payload[crypto_box_BEFORENMBYTES];
//	unsigned char sharedkey_envelope[crypto_box_BEFORENMBYTES]

	if ( encrypted == _CHATPACKET_ENCRYPTED ) {
		crypto_box_beforenm(
			(unsigned char *)&sharedkey_payload, 
			(unsigned char *)&(config->peerkeys_payload_public), 
			(unsigned char *)&(config->payloadkeys_private)
		);
		
		printf( " === > Shared Key: ");
		print_bin2hex((unsigned char *)&sharedkey_payload, sizeof(sharedkey_payload) );
	}


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


	p_length = cp->payloadLength + 16 + 1 + 4;
	
	
	if ( encrypted == _CHATPACKET_ENCRYPTED ) {
		// encryption overhead + tag+payload length
		payload=calloc(p_length+5,sizeof(unsigned char));
		
		p_length_encrpyted = p_length + crypto_aead_chacha20poly1305_ABYTES + 5; 
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

		p_length+=5;
		
	} else {
		payload=calloc(p_length,sizeof(unsigned char));
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
	assert( i <= p_length );
	
	if ( encrypted == _CHATPACKET_ENCRYPTED ) {

			// crypto_box_easy(ciphertext, MESSAGE, MESSAGE_LEN, nonce, bob_publickey, alice_secretkey);
			crypto_box_easy(
				payload_encrypted, 
				payload, p_length, 
				(const unsigned char *)&cp->payloadNonce, 
				(unsigned char *)&(config->peerkeys_payload_public), 
				(unsigned char *)&(config->payloadkeys_private)
			);
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

/* 
***********************************
	Envelope 
***********************************
*/	

// 1 + 24 + 32 + 32 + 28 + 12(tags) = 129

	if ( encrypted == _CHATPACKET_ENCRYPTED ) {
		e_length = 129 + p_length_encrpyted + 1;
	} else {
		e_length = 129 + p_length;	
	}
	cp->envelopeLength = e_length;
	
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

// msgid
	//tag
	c = cptag_msgid;
	memcpy(envelope+i, &c, 1);
	++i;	
	//data
	ni = htonl(cp->msgid);
	memcpy(envelope+i, &ni, 4);
	i += 4;

// flags
	//tag
	c = cptag_flags;
	memcpy(envelope+i, &c, 1);
	++i;	
	//data
	ni = htonl(cp->flags);
	memcpy(envelope+i, &ni, 4);
	i += 4;

// UUID TO:
	//tag
	c = cptag_to0;
	memcpy(envelope+i, &c, 1);
	++i;	
	// data
	uuid_enc_be(envelope+i, &cp->to0);
	i += 16;
	
	//tag
	c = cptag_to1;
	memcpy(envelope+i, &c, 1);
	++i;	
	// data	
	uuid_enc_be(envelope+i, &cp->to1);
	i += 16;

// UUID FROM:
	
	//tag
	c = cptag_from0;
	memcpy(envelope+i, &c, 1);
	++i;	
	//data	
	uuid_enc_be(envelope+i, &cp->from0);
	i += 16;
	
	//tag
	c = cptag_from1;
	memcpy(envelope+i, &c, 1);
	++i;	
	//data		
	uuid_enc_be(envelope+i, &cp->from1);
	i += 16;

// PayLoad Key:	
	//tag
	c = cptag_payloadKey;
	memcpy(envelope+i, &c, 1);
	++i;	
	// data	
	uuid_enc_be(envelope+i, &cp->payloadKey);
	i += 16;

// PayLoad Nonce:
	//tag
	c = cptag_payloadNonce;
	memcpy(envelope+i, &c, 1);
	++i;
	// data
	ni = htonl( (cp->payloadNonce & 0xFFFFFFFF00000000) >> 32 );
	memcpy(envelope+i, &ni, 4);
	i += 4;
	ni = htonl( (cp->payloadNonce) & 0x00000000FFFFFFFF );
	memcpy(envelope+i, &ni, 4);
	i += 4;
	
// Payload Length
	//tag
	c = cptag_payloadLength;
	memcpy(envelope+i, &c, 1);
	++i;	
	//data
	if ( encrypted == _CHATPACKET_ENCRYPTED ) {
		ni = htonl(p_length_encrpyted);
	} else {
		ni = htonl(cp->payloadLength);
	}
	memcpy(envelope+i, &ni, 4);
	i += 4;

// Payload Length
	//tag
/*	
	c = cptag_encryptedEnvelope;
	memcpy(envelope+i, &c, 1);
	++i;	
*/

// copy payload into envelope

	if ( encrypted == _CHATPACKET_ENCRYPTED ) {
	// Tag
		c = cptag_encryptedPayload;
		memcpy(envelope+i, &c, 1);
		++i;
		memcpy(envelope+i, payload_encrypted, p_length_encrpyted);
		i += p_length_encrpyted;
		free(payload);
		free(payload_encrypted);
	} else {
		memcpy(envelope+i, payload, p_length);
		i += p_length;
		free(payload);
	}
	

// Payload Random Padding - Low
	// Tag
	c = cptag_envelopeRandomPaddingLow;
	memcpy(envelope+i, &c, 1);
	++i;
	// data
	memcpy(envelope+i, &(cp->envelopeRandomPadding[h]), l);
	i += l;
	

	if ( i != e_length ) {	
		printf ( " WARNING = Envelope => index didn't match e_length! %u != %u\n", i, e_length );	
	}
	assert( i <= e_length );
	
	ob->length = e_length+31;
	ob->msg = calloc(ob->length,sizeof(unsigned char));;
	i=0;

// PayLoad Key:	
	//tag
	c = cptag_envelopeKey;
	memcpy(ob->msg+i, &c, 1);
	++i;	
	// data	
	uuid_enc_be(ob->msg+i, &cp->envelopeKey);
	i += 16;

// envelopeNonce Nonce:
	//tag
	c = cptag_envelopeNonce;
	memcpy(ob->msg+i, &c, 1);
	++i;
	// data

	ni = htonl( (cp->envelopeNonce & 0xFFFFFFFF00000000) >> 32 );
	memcpy(ob->msg+i, &ni, 4);
	i += 4;

	ni = htonl( cp->envelopeNonce & 0x00000000FFFFFFFF );
	memcpy(ob->msg+i, &ni, 4);
	i += 4;

// Payload Length
	//tag
	c = cptag_envelopeLength;
	memcpy(ob->msg+i, &c, 1);
	++i;	
	//data
	ni = htonl(cp->envelopeLength);
	memcpy(ob->msg+i, &ni, 4);
	i += 4;

	memcpy(ob->msg+i, envelope, e_length);
	free(envelope);
	i += e_length;
	assert( i <= ob->length );
	
		
}


// FIXME: should add size_t len, and assert ( b+x < b+len )
void chatPacket_decode (chatPacket *cp, unsigned char *b, ssize_t len, chatFabricConfig *config) {
	uint32_t ni=0, i=0, length = 0;
	unsigned char c=0, h=0, l=0, hp=0, lp = 0;
	uint64_t ni64 =0;
	unsigned char sharedkey_payload[crypto_box_BEFORENMBYTES];
	unsigned char *decrypted=0;
	unsigned long long decrypted_len=0;

	
	while (i<len) {
		memcpy(&c, b+i, 1);
		++i;
		switch (c){
			case cptag_envelopeKey:
				uuid_dec_be(b+i, &cp->envelopeKey);
				i+=16;
			break;

			case cptag_envelopeNonce:
				memcpy(&ni, b+i, 4);
				i+=4;
				ni = ntohl(ni);
				ni64 = ni;
				ni64 = ( ni64 << 32 );

				memcpy(&ni, b+i, 4);
				i+=4;
				ni = ntohl(ni);
				cp->envelopeNonce =  ni64 | ni;				
			break;
			
			case cptag_envelopeLength:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->envelopeLength = ntohl(ni);
			break;				
			case cptag_encryptedEnvelope:
				// TBD
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
			case cptag_msgid:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->msgid = ntohl(ni);			
			break;
			case cptag_flags:
				memcpy(&ni, b+i, 4);
				i+=4;
				cp->flags = ntohl(ni);			
			break;
			
			case cptag_payloadKey:
				uuid_dec_be(b+i, &cp->payloadKey);
				i+=16;
			break;
			case cptag_payloadNonce:
				memcpy(&ni, b+i, 4);
				i+=4;
				ni = ntohl(ni);
				ni64 = ni;
				ni64 = ( ni64 << 32 );

				memcpy(&ni, b+i, 4);
				i+=4;
				ni = ntohl(ni);
				cp->payloadNonce =  ni64 | ni;				
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
			break;
			case cptag_payloadRandomPaddingHigh:
				assert ( hp != 0 ); // zero length is invalid padding.
				memcpy(&(cp->payloadRandomPadding), b+i, hp);
				i+=hp;
			break;			
			
			case cptag_payloadRandomPaddingLow:
				assert ( lp != 0 ); // zero length is invalid padding.
				memcpy(&(cp->envelopeRandomPadding[hp]), b+i, lp);
				i+=lp;
			break;			
			case cptag_encryptedPayload:
				memcpy(cp->payload, b+i, cp->payloadLength);
				decrypted=calloc(cp->payloadLength,sizeof(unsigned char));	
				
				
				crypto_box_beforenm(
					(unsigned char *)&sharedkey_payload, 
					(unsigned char *)&(config->peerkeys_payload_public), 
					(unsigned char *)&(config->payloadkeys_private)
				);

				printf( " === > Shared Key: ");
				print_bin2hex((unsigned char *)&sharedkey_payload, sizeof(sharedkey_payload) );

	//

/*					crypto_aead_chacha20poly1305_decrypt(
						decrypted, &decrypted_len,
						NULL,
						b+i, cp->payloadLength,
						NULL,
						0,
						(const unsigned char *)&cp->payloadNonce, 
						(const unsigned char *)&sharedkey_payload) == 0) 
*/


				if ( crypto_box_open_easy(
						decrypted, 
						b+i, cp->payloadLength, 
						(const unsigned char *)&cp->payloadNonce,  
						(unsigned char *)&(config->peerkeys_payload_public), 
						(unsigned char *)&(config->payloadkeys_private)
				) == 0 )
				{
					chatPacket_decode (cp, decrypted, decrypted_len, config);
					// cp->payloadLength = decrypted_len;
					
				} else {
					printf( " ===========> decryption failed!\n");
				}

				i+=cp->payloadLength;

			break;
			case cptag_payload:
				memcpy(cp->payload, b+i, cp->payloadLength);
				i+=cp->payloadLength;		
			break;
			default:				
				printf ( " == BAD CHAT PACKET =>> Last 6 bytes %02x %02x %02x %02x %02x %02x \n ", b[i-2],  b[i-1], b[i], b[i+1], b[i+2], b[i+3]);
				return;
				++i;
			break;
		}
	}

}

void chatPacket_print (chatPacket *cp) {

	uint32_t status,i;
	char *str;
	char p;

	printf ( " === ChatPacket ========================================== \n");
	printf ( " %24s %s\n", "Field", "Value");
	printf ( " %24s %s\n", "---------", "---------");

	uuid_to_string(&cp->envelopeKey, &str, &status);
	printf ( " %24s: %42s\n", "envelopeKey", str);
	free(str);
	
	printf ( " %24s: %42lx\n", "envelopeNonce", cp->envelopeNonce);
	printf ( " %24s: %42u\n", "envelopeLength", cp->envelopeLength);

	printf ( " %24s: %42x\n", "envelope Padding Length", cp->envelopeRandomPaddingLength);
	printf ( " %24s: ", "envelope Padding");
	print_bin2hex((unsigned char *)&cp->envelopeRandomPaddingLength, 16);


	uuid_to_string(&cp->to0, &str, &status);
	printf ( " %24s: %42s\n", "to0", str);
	free(str);
	uuid_to_string(&cp->to1, &str, &status);
	printf ( " %24s: %42s\n", "to1", str);
	free(str);

	uuid_to_string(&cp->from0, &str, &status);
	printf ( " %24s: %42s\n", "from0", str);
	free(str);
	uuid_to_string(&cp->from1, &str, &status);
	printf ( " %24s: %42s\n", "from1", str);
	free(str);

	printf ( " %24s: %42d\n", "Message ID", cp->msgid);
	printf ( " %24s: %42x\n", "flags", cp->flags);

	uuid_to_string(&cp->payloadKey, &str, &status);
	printf ( " %24s: %42s\n", "payloadKey", str);
	free(str);

	printf ( " %24s: %42lx\n", "payloadNonce", cp->payloadNonce);
	printf ( " %24s: %42d\n", "PayLoadLength", cp->payloadLength);

	printf ( " %24s: %42x\n", "payload Padding Length", cp->payloadRandomPaddingLength);
	printf ( " %24s: ", "envelope Padding");
	print_bin2hex((unsigned char *)&cp->payloadRandomPadding, 16);
	
	printf ( " %24s: ", "payload" );

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
