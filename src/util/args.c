#ifndef ESP8266
#ifndef IOS_APP
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "args.h"


void CP_ICACHE_FLASH_ATTR
chatFabric_usage(char *p) {
	printf ("%s -config configfile\n", p);
	printf ("   -c --config   FILE           config file to use.\n");
	printf ("   -P --pairfile FILE           pairing binary data.\n");
	printf ("   -i --ip IPADDRESS           ip address.\n");
	printf ("   -d --debug                  ip address.\n");
	printf ("   -p --port                   port number.\n");
	printf ("   -k --genkeys                generate public/private key pair.\n");

	printf ("      --to0                    Namespace UUID, Send TO (uuid0).\n");
	printf ("      --to1                    Instance UUID, Send TO (uuid1).\n");

	printf ("   -u --uuid0                  Namespace UUID (uuid0).\n");
	printf ("   -v --uuid1                  Instance UUID (uuid1).\n");

	printf ("   -z --genuuid1               generate uuid1, set uuid0 to NIL/zeros.\n");
	printf ("   -w --writeconfig FILE       new configuration file to write\n");
	printf ("   -m --message STRING         Send message in payload.\n");

	printf ("   --set \n");
	printf ("   --get \n");
	printf ("   --send \n");
	printf ("   --control \n");
	printf ("   --value \n");
	printf ("   --tcp \n");
	printf ("   --udp \n");

	return;
}

void CP_ICACHE_FLASH_ATTR
chatFabric_args(int argc, char**argv, chatFabricConfig *config, chatFabricAction *a) {
	int ch;
	uint32_t status;
	static const unsigned char basepoint[32] = {9};

	config->configfile = NULL;
	config->newconfigfile = NULL;
	config->pairfile = NULL;
	config->ip = NULL;
	config->port = 32000;	
	config->debug = 0;
	config->writeconfig = 0;
	config->type = SOCK_DGRAM;
	
	struct option longopts[] = {
		{	"config",	required_argument,	NULL,	'c'	},
		{	"pairfile",		required_argument,	NULL,	'P'	},
		{	"debug",	no_argument,		NULL,	'd'	},
		{	"ip",		required_argument,	NULL,	'i'	},
		{	"port",		required_argument,	NULL,	'p'	},
		{	"genkeys",	no_argument,		NULL,	'k'	},

		{	"to0",	required_argument,	NULL,	'a'	},
		{	"to1",	required_argument,	NULL,	'b'	},

		{	"uuid0",	required_argument,	NULL,	'u'	},
		{	"uuid1",	required_argument,	NULL,	'v'	},

		{	"genuuid1",	no_argument,		NULL,	'z'	},
		{	"writeconfig",	required_argument,		NULL,	'w'	},
		{	"message",	required_argument,		NULL,	'm'	},		

		{	"set" ,	no_argument,	NULL,	0},
		{	"get" ,	no_argument,	NULL,	1},	
		{	"send",	no_argument,	NULL,	2},	
		
		{	"control",	required_argument,	NULL, 3},		
		{	"value",	required_argument,	NULL,	 4},		
		{	"tcp",	no_argument,	NULL,	 5},		
		{	"udp",	no_argument,	NULL,	 6},		

		/*  remember a zero line, else 
			getopt_long segfaults with unknown options */
	    {NULL, 			0, 					0, 		0	}
			
	};

	
	while ((ch = getopt_long(argc, argv, "a:b:c:di:kp:s:u:v:zw:P:m:", longopts, NULL)) != -1) {
		switch (ch) {

			case 0:
				a->action = (uint32_t )ACTION_SET;
			break;
			case 1:
				a->action = (uint32_t )ACTION_GET;
			break;
			case 2:
				a->action = (uint32_t )ACTION_READ;
			break;
			case 3:
				a->action_control = (uint32_t )atoi(optarg);
			break;
			case 4:
				a->action_value = (uint32_t )atoi(optarg);
			break;
			case 5:
				config->type = SOCK_STREAM;
			break;
			case 6:
				config->type = SOCK_DGRAM;
			break;
							
			break;
			case 'a':
				uuid_from_string(
					optarg, 
					&(config->to.u0), 
					&status);
			break;
			case 'b':
				uuid_from_string(
					optarg, 
					&(config->to.u1), 
					&status);	
			break;

			case 'c':
				//printf ( "Arg --config : Value : %s \n", optarg );
				config->configfile = optarg;
			break;
			case 'd':
				//printf ( "Arg --debug : Value : %s \n", optarg );
				config->debug = 1;
			break;
			case 'i':
				//printf ( "Arg --ip : Value : %s \n", optarg );
				config->ip = optarg;
			break;
			case 'p':
				//printf ( "Arg --ip : Value : %s \n", optarg );
				config->port = atoi(optarg);
			break;
			case 'P':
				//printf ( "Arg --ip : Value : %s \n", optarg );
				config->pairfile =	optarg;
			break;
			case 'u':
				uuid_from_string(
					optarg, 
					&(config->uuid.u0), 
					&status);
			break;
			case 'v':
				uuid_from_string(
					optarg, 
					&(config->uuid.u1), 
					&status);			
			break;
			case 'k':
				#ifdef HAVE_SODIUM			
				crypto_box_keypair((unsigned char *)&(config->publickey), (unsigned char *)&(config->privatekey));
				#endif 
				#ifdef HAVE_LOCAL_CRYPTO
				arc4random_buf((unsigned char *)&(config->privatekey), crypto_box_SECRETKEYBYTES);
				curve25519_donna((unsigned char *)&config->publickey, (unsigned char *)&config->privatekey, (unsigned char *)&basepoint);
				#endif

			break;
			case 'z':
				uuid_create_nil(&(config->uuid.u0),  &status);
				uuid_create(&(config->uuid.u1),  &status);
			break;
			case 'w':
				//printf ( "Arg --config : Value : %s \n", optarg );
				config->newconfigfile = optarg;
				config->writeconfig = 1;				
			break;			
			case 'm':
				//printf ( "Arg --ip : Value : %s \n", optarg );
				config->msg = (unsigned char *)optarg;
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
#endif
#endif