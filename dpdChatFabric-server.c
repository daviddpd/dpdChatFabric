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


#define CHILDREN 1
#define PORTNUMBER 32000

void pp(unsigned char * x) {
	int i;
	for (i=0; i<32; i++) {
		printf ( "%02x", x[i] );
	}
	printf ("\n");

}


int main(int argc, char**argv)
{
	int sockfd,n;
	struct sockaddr_in servaddr,cliaddr;	
	int kq;
	struct kevent evSet;
	struct kevent evList[32];
	int nev, i,x;
	int lowwater = 32;
	pid_t pid;
	pid_t childpid[CHILDREN];
	chatPacket *cp, *cp_reply;
	int status;
	uint32_t status2;
	chatFabricPairing pair;
	size_t  sentbytes;
	msgbuffer mb;

	uuid_tuple to;

	socklen_t len;
	unsigned char *mesg;
	char *str;	
	chatFabricConfig config;
	enum chatPacketCommands replyCmd;
	enum chatPacketPacketTypes cptype;

	mesg=calloc(1400,sizeof(unsigned char));

	pair.state = STATE_UNCONFIGURED;
	pair.hasPublicKey = 0;
	
	uuid_create_nil ( &(pair.uuid.u0), &status2);
	uuid_create_nil ( &(pair.uuid.u1), &status2);
	pair.hasPublicKey = 0;
	pair.hasNonce = 0;
//	arc4random_buf(&(pair.nonce), crypto_secretbox_NONCEBYTES);
	arc4random_buf(&(pair.mynonce), crypto_secretbox_NONCEBYTES);
	bzero(&(pair.nonce), crypto_secretbox_NONCEBYTES);

	for (i=0; i<crypto_box_PUBLICKEYBYTES; i++) {
		pair.publickey[i] = 0;
		config.publickey[i] = 0;
		config.privatekey[i] = 0;
	}


	chatFabric_args(argc, argv, &config);
	
	chatFabric_configParse(&config);
	uuid_to_string(&(config.uuid.u0), &str, &status2);
	printf (" uuid0       : %s\n", str);
	uuid_to_string(&(config.uuid.u1), &str, &status2);
	printf (" uuid1       : %s\n", str);


	printf ( " %-16s ==> ", "PUBLICKEY" );	
	print_bin2hex((unsigned char *)&(config.publickey), crypto_box_PUBLICKEYBYTES);
	printf ( " %-16s ==> ", "PRIVATEKEY" );	
	print_bin2hex((unsigned char *)&(config.privatekey), crypto_box_PUBLICKEYBYTES);


	uuid_create_nil(&to.u0, &status2);
	uuid_create_nil(&to.u1, &status2);

	
	// Open UDP datagram socket
	// Pretty Standard Socket binding code

	sockfd=socket(AF_INET,SOCK_DGRAM,0);	
	bzero(&servaddr,sizeof(servaddr));	
	
	setsockopt(sockfd, SOL_SOCKET, SO_RCVLOWAT, &lowwater, sizeof(lowwater));
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(PORTNUMBER);
	bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	
	CHATFABRIC_DEBUG(config.debug, "testing" );
	
	// Prefork the children 
	
	for (x=0; x<CHILDREN; x++) {
		childpid[x] = fork();
	
	
		if (childpid[x] == 0) { 
		
			// In Child
			kq = kqueue();
			pid = getpid();			
			
			// Setup the Kevent Event.
			EV_SET(&evSet, sockfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
			if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)  {
				printf("erorr kevent\n");
				exit(1);
			}

			for (;;){
				len = sizeof(cliaddr);
				// Wait for the 
				nev = kevent(kq, NULL, 0, evList, 32, NULL);
				if (nev < 1) { 
					printf("error kevent\n");
				} else {
								
					for (i=0; i<nev; i++) { 
						if (evList[i].ident == sockfd) {
							printf ("\n\n============================  Listening  ====================\n");
							//printf ( "\n\n === > Got Packet\n" );
							cp = chatPacket_init0 ();
							cp_reply = chatPacket_init0 ();
							
							n = recvfrom(sockfd,mesg,1400,0,(struct sockaddr *)&cliaddr,&len);
							if ( chatPacket_decode (cp, &pair, mesg, n, &config ) != 0 ) {
								printf ("             =============  chatPacket Decoding failed  ====================\n");
								exit(1);
							}
							chatPacket_print(cp, IN);
							replyCmd = stateMachine ( &config, cp, &pair, cp_reply );
							if ( replyCmd == CMD_SEND_REPLY_TRUE ) {
//								sleep(1);
								printf ("             =============  Sending Reply  ====================\n");

								chatPacket_print( cp_reply, OUT);
								
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
									(struct sockaddr *)&cliaddr,len);
								free(mb.msg);
								mb.length = 0;
								chatPacket_delete(cp);
									
							} else {
								printf ("===> CMD_SEND_REPLY_FALSE\n");
			
							}
							printf ("================================================================================== \n");			
							chatPacket_delete(cp);							
						} else {
							printf ("[%d] Not the socket id.\n", pid);    	
						}
					}
				
			}
		} 
		
		exit(0);
	}
} 

	printf("PARENT PID: %d\n", getpid());
	for (x=0; x<32; x++) { 	
//		printf("[%d] Waiting for Child PID: %d\n", getpid(), childpid[x]);
		waitpid(childpid[x], &status, 0 ); 
	}
}
