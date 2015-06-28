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


void 
chatFabric_hex2int_bytes (unsigned char *hex, uint32_t hexLength, unsigned char *dst, uint32_t dstLenght ) {
	char str[2];
	int i=0,x=0;
	uint32_t dst_int;
	//str[2] = 0;
	printf ( "==> chatFabric_hex2int_bytes\n" );
	for ( i = 0; i<hexLength; i=i+2) {
		str[0] = hex[i];
		str[1] = hex[i+1];
		if ( x < dstLenght-1 ) {
			sscanf (str, "%d", &dst_int );
			//dst_int = strtoul(str, NULL, 16);		
			dst[x] = dst_int;
		}
		
		printf ( " === %2s ?= %02x = %4d = ===========\n", str, dst[x], dst_int );
		
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
	const ucl_object_t *obj = NULL, *cur = NULL;
	ucl_object_iter_t it = NULL, it2 = NULL;
	const char *key;
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
		&config->payloadkeys_public,
		sizeof(config->payloadkeys_public)
		);
	
	obj = ucl_lookup_path(root, "payloadkeys.public");
	config->payloadkeys_public_str =	(unsigned char *)ucl_object_tostring(obj);

	obj = ucl_lookup_path(root, "envelopekeys.private");
	config->envelopekeys_private_str = (unsigned char *)ucl_object_tostring(obj);
	
	obj = ucl_lookup_path(root, "envelopekeys.public");
	config->envelopekeys_public_str =	(unsigned char *)ucl_object_tostring(obj);


	obj = ucl_lookup_path(root, "peerkeys.payloadpublic");
	config->peerkeys_payload_public_str =	(unsigned char *)ucl_object_tostring(obj);

	obj = ucl_lookup_path(root, "peerkeys.envelopepublic");
	config->peerkeys_envelope_public_str =	(unsigned char *)ucl_object_tostring(obj);

	arc4random_buf(&config->envelopeNonce, 8);
	arc4random_buf(&config->payloadNonce, 8);

}


uint32_t 
chatPacket_encode (chatPacket *cp, unsigned char *b, chatFabricConfig *config ) {
	uint32_t length =0, offset_padding = 0;
	uint32_t ni;
	uint32_t ni64;
	unsigned char *tmp;
	unsigned char paddingLength, h, l;

	uuid_enc_be(b+OFFSET_ENVELOPEKEY, &cp->envelopeKey);

	ni64 = htonl(config->envelopeNonce);
	memcpy(b+OFFSET_ENVELOPENONCE, &ni64, 8);
	config->envelopeNonce++;

	h =  arc4random_uniform(15) + 1;
	l = 16 - h;
	paddingLength = (h << 4) | l;

//	arc4random_buf(&b, 16);

	memcpy(b+OFFSET_TO0, &paddingLength, 1);
	offset_padding++;
	arc4random_buf(b+OFFSET_TO0+offset_padding, h);
	offset_padding+=h;	

	uuid_enc_be(b+OFFSET_TO0+offset_padding, &cp->to0);
	uuid_enc_be(b+OFFSET_TO1+offset_padding, &cp->to1);
	uuid_enc_be(b+OFFSET_FROM0+offset_padding, &cp->from0);
	uuid_enc_be(b+OFFSET_FROM1+offset_padding, &cp->from1);
	
	ni = htonl(cp->msgid);
	memcpy(b+OFFSET_MSGID+offset_padding, &ni, 4);

	ni = htonl(cp->flags);
	memcpy(b+OFFSET_FLAGS+offset_padding, &ni, 4);
	
	uuid_enc_be(b+OFFSET_PAYLOADKEY+offset_padding, &cp->payloadKey);
	
	ni = htonl(cp->payloadLength);
	memcpy(b+OFFSET_PAYLOADLENGTH+offset_padding, &ni, 4);
	
	ni64 = htonl(config->payloadNonce);
	memcpy(b+OFFSET_PAYLOADNOCE+offset_padding, &ni64, 8);
	config->payloadNonce++;
	
	
	memcpy(b+OFFSET_PAYLOAD+offset_padding, &(cp->payload), cp->payloadLength);

	length = OFFSET_PAYLOAD - 1 + cp->payloadLength + offset_padding + l;

	cp->envelopeLength = (length+offset_padding) - OFFSET_TO0 - 1;

	ni = htonl(cp->envelopeLength);
	memcpy(b+OFFSET_ENVELOPELENGTH, &ni, 4);

	arc4random_buf(b+OFFSET_PAYLOAD+offset_padding+cp->payloadLength, l);
	
//	(char *) malloc( cp->payloadLength * sizeof(char) );

	
	return length;
	
}


// FIXME: should add size_t len, and assert ( b+x < b+len )
void chatPacket_decode (chatPacket *cp, unsigned char *b, chatFabricConfig *config) {
	uint32_t ni, offset_padding = 0;
	unsigned char paddingLength, h, l;
	uint64_t ni64;

	uuid_dec_be(b+OFFSET_ENVELOPEKEY, &cp->envelopeKey);

	memcpy(&ni, b+OFFSET_ENVELOPELENGTH, 4);
	cp->envelopeLength = ntohl(ni);

	memcpy(&ni64, b+OFFSET_ENVELOPENONCE, 8);
	cp->envelopeNonce = ntohl(ni64);

	memcpy(&paddingLength, b+OFFSET_TO0, 1);
	offset_padding++;
	h = paddingLength ^ 0x0F;
	h = h >> 4;
	l = paddingLength ^ 0xF0;
	offset_padding+=h;

	uuid_dec_be(b+OFFSET_TO0+offset_padding, &cp->to0);
	uuid_dec_be(b+OFFSET_TO1+offset_padding, &cp->to1);
	uuid_dec_be(b+OFFSET_FROM0+offset_padding, &cp->from0);
	uuid_dec_be(b+OFFSET_FROM1+offset_padding, &cp->from1);
	uuid_dec_be(b+OFFSET_PAYLOADKEY+offset_padding, &cp->payloadKey);
	
	memcpy(&ni,b+OFFSET_MSGID+offset_padding, 4);
	cp->msgid = ntohl(ni);

	memcpy( &ni,b+OFFSET_FLAGS+offset_padding, 4);
	cp->flags = ntohl(ni);
	
	memcpy(&ni,b+OFFSET_PAYLOADLENGTH+offset_padding, 4);	
	cp->payloadLength = ntohl(ni);

	memcpy(&ni64, b+OFFSET_PAYLOADNOCE+offset_padding, 8);
	cp->payloadNonce = ntohl(ni64);

	
//	length = 112 + cp->payloadLength;
	memcpy(&(cp->payload), b+OFFSET_PAYLOAD+offset_padding, cp->payloadLength);

	

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
	
	printf ( " %24s: %42u\n", "envelopeNonce", cp->envelopeNonce);
	printf ( " %24s: %42u\n", "envelopeLength", cp->envelopeLength);

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

	printf ( " %24s: %42u\n", "payloadNonce", cp->payloadNonce);

	printf ( " %24s: %42d\n", "PayLoadLength", cp->payloadLength);
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
