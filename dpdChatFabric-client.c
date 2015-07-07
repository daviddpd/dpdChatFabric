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



#define PORTNUMBER 32000


int main(int argc, char**argv)
{
	int sockfd,pid;
//	chatPacket cp;
	chatPacket *cp2;
//	unsigned char buffer[1400];
	unsigned char *random;

	msgbuffer mb;
	struct sockaddr_in servaddr;
	uint32_t status, randomLength;
	size_t  sentbytes;
	chatFabricConfig config;
	char *str;
	uuid_tuple to;
	char *_lyrics = "You lit the fire  Then drank the water  You slammed that door and left me standing all alone  We wrote the story  We turned the pages  You changed the end like everybody said you would    I should have seen it coming  It should have sent me running  That's what I get for loving you    If I had a time machine and  If life was a movie scene  I'd rewind and I'd tell me run  We were never meant to be  So if I had a time machine  I'd go back and I'd tell me run run    - TIME MACHINE, WRITTEN BY INGRID MICHAELSON, BUSBEE, TRENT DABBS";

	random=calloc(534,sizeof(unsigned char));
	
/*	if (argc != 2)
	{
	  printf("usage:  udp-client <IP address>\n");
	  exit(1);
	}
*/	
	chatFabric_args(argc, argv, &config);
	
	chatFabric_configParse(&config);
	uuid_to_string(&(config.uuid0), &str, &status);
	printf (" uuid0       : %s\n", str);
	uuid_to_string(&(config.uuid1), &str, &status);
	printf (" uuid1       : %s\n", str);

	printf ("p public key  : %s\n", config.payloadkeys_public_str );
	printf ("p private key : %s\n", config.payloadkeys_private_str );
	printf ("e public key  : %s\n", config.envelopekeys_public_str );
	printf ("e private key : %s\n", config.envelopekeys_private_str );

	
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=inet_addr(config.ip);
	servaddr.sin_port=htons(PORTNUMBER);
	
	pid = getpid();
	
	uuid_from_string(_UUID0, &to.u0, &status);
	uuid_from_string("ca32aa22-0e7c-11e5-8e8b-00a0988afcc9", &to.u1, &status);
	
	randomLength = 534;
	memcpy(random, _lyrics, randomLength);
	random[randomLength-1] = 0x00;
	
	while (1)
	{
	
		printf ( " ==> chatPacket_init ... \n");
		cp2 = chatPacket_init (&config, &to,  random, randomLength,  0);
		printf ( " ==> chatPacket_init done \n");

//		chatPacket_encode ( cp2, &config, &mb, _CHATPACKET_ENCRYPTED);
		chatPacket_encode ( cp2, &config, &mb, _CHATPACKET_CLEARTEXT);


		sentbytes = sendto(sockfd,mb.msg,mb.length,0,
			(struct sockaddr *)&servaddr,sizeof(servaddr));
		

		chatPacket_print(cp2);
		free(mb.msg);
		mb.length = 0;
		chatPacket_delete(cp2);
		sleep(10);	
	}
}

