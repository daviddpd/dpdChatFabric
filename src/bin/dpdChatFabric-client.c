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



#define PORTNUMBER 1080


int main(int argc, char**argv)
{
	int sockfd,pid;
//	chatPacket cp;
	chatPacket *cp, *cp2, *cp_reply;
//	unsigned char buffer[1400];
	unsigned char *random;
	unsigned char *mesg;
	int lowwater = 32;

	msgbuffer mb;
	struct sockaddr_in servaddr;
	uint32_t status, randomLength;
	size_t  sentbytes, n;
	chatFabricConfig config;
	char *str;
	uuid_tuple to;
	char *_lyrics = "You lit the fire  Then drank the water  You slammed that door and left me standing all alone  We wrote the story  We turned the pages  You changed the end like everybody said you would    I should have seen it coming  It should have sent me running  That's what I get for loving you    If I had a time machine and  If life was a movie scene  I'd rewind and I'd tell me run  We were never meant to be  So if I had a time machine  I'd go back and I'd tell me run run    - TIME MACHINE, WRITTEN BY INGRID MICHAELSON, BUSBEE, TRENT DABBS";
	unsigned char * nullmsg = 0;
	socklen_t len;
	enum chatPacketCommands replyCmd;
	enum chatPacketPacketTypes cptype;
	
	chatFabricPairing pair;


	pair.state = STATE_UNCONFIGURED;
	pair.hasPublicKey = 0;
	pair.hasNonce = 0;
	
	random=calloc(534,sizeof(unsigned char));
	mesg=calloc(1400,sizeof(unsigned char));
	
/*	if (argc != 2)
	{
	  printf("usage:  udp-client <IP address>\n");
	  exit(1);
	}
*/	
	chatFabric_args(argc, argv, &config);
	
	chatFabric_configParse(&config);
	uuuid2_to_str(&str, 16, &(config.uuid.u0));	
	printf (" uuid0       : %s\n", str);
	uuuid2_to_str(&str, 16, &(config.uuid.u1));
	printf (" uuid1       : %s\n", str);
	
	printf ( " %-16s ==> ", "PUBLICKEY" );	
	print_bin2hex((unsigned char *)&(config.publickey), crypto_box_PUBLICKEYBYTES);
	printf ( " %-16s ==> ", "PRIVATEKEY" );	
	print_bin2hex((unsigned char *)&(config.privatekey), crypto_box_PUBLICKEYBYTES);
		
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=inet_addr(config.ip);
	servaddr.sin_port=htons(config.port);

	setsockopt(sockfd, SOL_SOCKET, SO_RCVLOWAT, &lowwater, sizeof(lowwater));
	
	pid = getpid();
	
	uuuid2_gen_nill(&to.u0);
	uuuid2_from_str("ca32aa22-0e7c-11e5-8e8b-00a0988afcc9", &to.u1);

	memcpy(&(pair.uuid.u0), &(to.u0), 16);
	memcpy(&(pair.uuid.u1), &(to.u1), 16);
	arc4random_buf(&(pair.mynonce), crypto_secretbox_NONCEBYTES);
	bzero(&(pair.nonce), crypto_secretbox_NONCEBYTES);
	
//	printf ( " %24s: %42lx\n", "nonce", pair.mynonce);


	
	randomLength = 534;
	memcpy(random, _lyrics, randomLength);
	random[randomLength-1] = 0x00;
	
	
	cp2 = chatPacket_init (&config, &pair, CMD_HELLO,  nullmsg, 0,  0);
	chatPacket_encode ( cp2, &config, &pair,  &mb, _CHATPACKET_ENCRYPTED, COMMAND);

	sentbytes = sendto(sockfd,mb.msg,mb.length,0,
		(struct sockaddr *)&servaddr,sizeof(servaddr));

	chatPacket_print(cp2, OUT);
	len = sizeof(servaddr);
	n = recvfrom(sockfd,mesg,1400,0,(struct sockaddr *)&servaddr,&len);
	cp_reply = chatPacket_init0 ();

	chatPacket_decode (cp_reply, &pair, mesg, n, &config );
	chatPacket_print(cp_reply, IN);
	printf ("====================\n");			

	free(mb.msg);
	mb.length = 0;
	chatPacket_delete(cp2);
	chatPacket_delete(cp_reply);

	cp2 = chatPacket_init (&config, &pair, CMD_PAIR_REQUEST,  nullmsg, 0,  0);
	chatPacket_encode ( cp2, &config, &pair,  &mb, _CHATPACKET_ENCRYPTED, COMMAND);

	sentbytes = sendto(sockfd,mb.msg,mb.length,0,
		(struct sockaddr *)&servaddr,sizeof(servaddr));
	free(mb.msg);
	mb.length = 0;
	chatPacket_delete(cp2);

	chatPacket_print(cp2, OUT);
	len = sizeof(servaddr);

	while (pair.state != STATE_PAIRED)
	{
		printf ("\n\n============================  Listening  ====================\n");

		cp = chatPacket_init0 ();
		cp_reply = chatPacket_init0 ();		
		n = recvfrom(sockfd,mesg,1400,0,(struct sockaddr *)&servaddr,&len);
		if ( chatPacket_decode (cp, &pair, mesg, n, &config ) != 0 ) {
			printf ("             =============  chatPacket Decoding failed  ====================\n");
			exit(1);				
		}
		chatPacket_print(cp, IN);
		 enum chatFabricErrors e;
		stateMachine ( &config, cp, &pair, cp_reply , &replyCmd, &e);
		if ( replyCmd == CMD_SEND_REPLY_TRUE ) {
			printf ("             =============  Sending Reply  ====================\n");

			switch (cp_reply->cmd) {
				case CMD_NONCE_SEND:
					cptype = NONCE;
				break;
				case CMD_PUBLICKEY_SEND:
					cptype = PUBLICKEY;
				break;
				default:
					cptype = COMMAND;
				break;
			}
			
			chatPacket_encode ( cp_reply, &config, &pair, &mb, _CHATPACKET_ENCRYPTED, cptype);
	
			sentbytes = sendto(sockfd,mb.msg,mb.length,0,
				(struct sockaddr *)&servaddr,len);
			chatPacket_print(cp_reply, OUT);
				
			free(mb.msg);
			mb.length = 0;
			
		} else {
			printf ("===> CMD_SEND_REPLY_FALSE\n");
		
		}
		printf ("================================================================================== \n");
		chatPacket_delete(cp);							
		chatPacket_delete(cp_reply);
			
	}

	while (1) {
		cp2 = chatPacket_init (&config, &pair, CMD_APP_MESSAGE,  random, randomLength,  0);
		chatPacket_encode ( cp2, &config, &pair,  &mb, _CHATPACKET_ENCRYPTED, DATA);

		sentbytes = sendto(sockfd,mb.msg,mb.length,0,
			(struct sockaddr *)&servaddr,sizeof(servaddr));

		chatPacket_print(cp2, OUT);
		len = sizeof(servaddr);
		n = recvfrom(sockfd,mesg,1400,0,(struct sockaddr *)&servaddr,&len);
		cp_reply = chatPacket_init0 ();

		chatPacket_decode (cp_reply, &pair, mesg, n, &config );
		chatPacket_print(cp_reply, IN);
		printf ("====================\n");			

		free(mb.msg);
		mb.length = 0;
		chatPacket_delete(cp2);
		chatPacket_delete(cp_reply);

		cp2 = chatPacket_init (&config, &pair, CMD_HELLO,  nullmsg, 0,  0);
		chatPacket_encode ( cp2, &config, &pair,  &mb, _CHATPACKET_ENCRYPTED, COMMAND);

		sentbytes = sendto(sockfd,mb.msg,mb.length,0,
			(struct sockaddr *)&servaddr,sizeof(servaddr));

		chatPacket_print(cp2, OUT);
		len = sizeof(servaddr);
		n = recvfrom(sockfd,mesg,1400,0,(struct sockaddr *)&servaddr,&len);
		cp_reply = chatPacket_init0 ();

		chatPacket_decode (cp_reply, &pair, mesg, n, &config );
		chatPacket_print(cp_reply, IN);
		printf ("====================\n");			

		free(mb.msg);
		mb.length = 0;
		chatPacket_delete(cp2);
		chatPacket_delete(cp_reply);


	}
		
}

