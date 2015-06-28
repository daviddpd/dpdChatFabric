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


#define CHILDREN 32
#define PORTNUMBER 32000

int main(int argc, char**argv)
{
	int sockfd,n;
	struct sockaddr_in servaddr,cliaddr;	
	int kq;
	struct kevent evSet;
	struct kevent evList[32];
	int nev, i,x,z;
	int lowwater = 100;
	pid_t pid;
	pid_t childpid[CHILDREN];
	chatPacket cp;
	uint32_t status;

	socklen_t len;
	unsigned char mesg[1400];	
	char *str;	
	
	chatFabricConfig config;

	for (i=0; i<32; i++) {
		config.payloadkeys_private[i] = 0;
	}


	chatFabric_args(argc, argv, &config);
	
	chatFabric_configParse(&config);
	uuid_to_string(&(config.uuid0), &str, &status);
	printf (" uuid0       : %s\n", str);
	uuid_to_string(&(config.uuid1), &str, &status);
	printf (" uuid1       : %s\n", str);

	printf ("p public key  : %s\n", config.payloadkeys_public_str );
	printf ("p private key : %s\n", config.payloadkeys_private_str );

	printf ("p private key : \n");
	printf (" [bin2hex]    : ");
	
	for (i=0; i<32; i++) {
		printf ( "%02x", config.payloadkeys_private[i] );
	}
	printf ("\n\n");
		
	printf ("e public key  : %s\n", config.envelopekeys_public_str );
	printf ("e private key : %s\n", config.envelopekeys_private_str );

	
	// Open UDP datagram socket
	// Pretty Standard Socket binding code

	sockfd=socket(AF_INET,SOCK_DGRAM,0);	
	bzero(&servaddr,sizeof(servaddr));	
	
	setsockopt(sockfd, SOL_SOCKET, SO_RCVLOWAT, &lowwater, sizeof(lowwater));
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(PORTNUMBER);
	bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	
	
	// Prefork the children 
	
	for (x=0; x<CHILDREN; x++) {
		childpid[x] = fork();
	
	
		if (childpid[x] == 0) { 
			// In Child
			kq = kqueue();
			pid = getpid();			
//			printf ("[%d] Child %d Forked\n", pid, x );
			
			// Setup the Kevent Event.
			EV_SET(&evSet, sockfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
			if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)  {
				printf("erorr kevent\n");
				exit(1);
			}

			for (;;){
//				printf("Loop [%d] \n", pid);
				len = sizeof(cliaddr);
				// Wait for the 
				nev = kevent(kq, NULL, 0, evList, 32, NULL);
				if (nev < 1) { 
					printf("error kevent\n");
				} else {
								
					for (i=0; i<nev; i++) { 
						if (evList[i].ident == sockfd) {      
							n = recvfrom(sockfd,mesg,1400,0,(struct sockaddr *)&cliaddr,&len);
/*							printf ("[%d] Packet Length: %d %d \n", pid, len, n);
							printf ("[%d] Packet Mesg  : \n", pid);
							for (z=0; z<n; z++) {
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
								printf ( "%02x ", mesg[z] );
							}
							printf ("\n");*/
							chatPacket_decode ( &cp, &mesg, &config );
							chatPacket_print(&cp);

							
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