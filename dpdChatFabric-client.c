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
	int sockfd,c=0,pid,z;
	chatPacket cp;
	unsigned char buffer[1400];
	unsigned char random[1000];
	struct sockaddr_in servaddr;
	uint32_t status, randomLength;
	size_t  length, sentbytes;
	chatFabricConfig config;
	char *str;
	
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
	
	uuid_from_string(_UUID0, &cp.payloadKey, &status);
	uuid_from_string(_UUID0, &cp.envelopeKey, &status);
	uuid_from_string(_UUID0, &cp.to0, &status);
	uuid_from_string("ca32aa22-0e7c-11e5-8e8b-00a0988afcc9", &cp.to1, &status);

	memcpy(&cp.from0, &config.uuid0, 16);
	memcpy(&cp.from1, &config.uuid1, 16);
		
	
	while (1)
	{
	
		randomLength =  arc4random_uniform(100);
		arc4random_buf(&random, randomLength);
		
	
		c++;
		bzero((void*)&cp.payload, sizeof(cp.payload));		
		cp.payloadLength = snprintf ( 
			(char*)&cp.payload, 
			sizeof(cp.payload), 
			"Packet Number: %08d", c 
		);
		cp.payloadLength++;  // FIXME/Explain: off my one, memcpy clips the string otherwise.
		memcpy((char*)&cp.payload+cp.payloadLength, &random, randomLength);
		cp.payloadLength+=randomLength;
		cp.msgid = c;
		
//		printf ( "\nPayLoad: %s\n", cp.payload );
//		printf ( "Payload length: %d\n",  cp.payloadLength );

		length = chatPacket_encode ( &cp, &buffer, &config );		
		
//		printf ( "Length is %d\n", (int)length );

		sentbytes = sendto(sockfd,&buffer,length,0,
			(struct sockaddr *)&servaddr,sizeof(servaddr));
		
/*		printf ( "[%d] Packets Sent: %d \n", pid, c );	
		printf ( "[%d] Packets sent-length: %d \n", pid, (int)sentbytes );	
		printf ( "[%d] Packets Sent: %d \n", pid, c );	
		for (z=0; z<length; z++) {
			if ( 
				(z == 16) ||
				(z == 21) ||
				(z == 53) ||
				(z == 85) ||
				(z == 93) ||
				(z == 109) ||
				(z == 113) 
			){
				printf ("\n");
			}
			printf ( "%02x ", buffer[z] );
		}
		printf("\n");		
		chatPacket_print(&cp);
*/
		chatPacket_print(&cp);
		sleep(5);
	
	}
}

