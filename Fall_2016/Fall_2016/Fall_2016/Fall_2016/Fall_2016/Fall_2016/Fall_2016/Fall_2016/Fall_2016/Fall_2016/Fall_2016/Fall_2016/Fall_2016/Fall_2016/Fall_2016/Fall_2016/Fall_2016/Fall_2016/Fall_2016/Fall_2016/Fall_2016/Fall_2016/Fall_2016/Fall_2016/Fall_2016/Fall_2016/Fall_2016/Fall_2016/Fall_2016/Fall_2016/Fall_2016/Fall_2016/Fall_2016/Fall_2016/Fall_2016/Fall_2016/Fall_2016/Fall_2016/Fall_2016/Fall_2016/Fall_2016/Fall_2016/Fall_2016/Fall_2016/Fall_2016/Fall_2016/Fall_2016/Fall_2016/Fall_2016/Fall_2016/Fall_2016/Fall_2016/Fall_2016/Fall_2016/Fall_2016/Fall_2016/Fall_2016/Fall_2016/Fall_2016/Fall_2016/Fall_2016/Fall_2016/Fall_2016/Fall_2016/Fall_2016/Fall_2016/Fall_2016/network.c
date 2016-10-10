#include <stdio.h>     
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/select.h>
#include <string.h> 
#include <unistd.h>
#include <netdb.h>
//for DNS service - translate char * to IP addr
#include <netinet/in.h>
//To use sockaddr_in
#include <arpa/inet.h>
//to use useful methods
#include "Main.h"
#include "network.h"
//_________________________________________________________________________________
char* cli_usage =  
"USAGE:\n\n\
[HELP] - print out this instruction \n\n\
[DISPLAY] - display information\n[name, UBIT name, UB e-mail, IP address of the process, port this process is listening.\n\n\
[REGISTER <server IP> <port no>] - Client only\n Register itself on the server as a peer\n\n\
[CONNECT <destination> <port no>] - A client will have its TCP connection with other peer in the network\n\n\
[LIST] - Display numbered list of all the connections this process if part of\n\n\
[TERMINATE <connection id>] - terminate the connection listed under the specified number when LIST is used to display all connections\n\n\
[QUIT] - CLOSE all connections and terminate this process \n\n\
[SEND <Connection id> <message> - send message to host on the connectoin that is designated by the connection ID,\n[More than 100 characters won't send']\n\n";
char* serv_usage =  
"USAGE:\n\n\
[HELP] - print out this instruction \n\n\
[DISPLAY] - display information\n[name, UBIT name, UB e-mail, IP address of the process, port this process is listening.\n\n\
[REGISTER <server IP> <port no>] - Client only\n Register itself on the server as a peer\n\n\
[CONNECT <destination> <port no>] - A client will have its TCP connection with other peer in the network\n\n\
[LIST] - Display numbered list of all the connections this process if part of\n\n\
[TERMINATE <connection id>] - terminate the connection listed under the specified number when LIST is used to display all connections\n\n\
[QUIT] - CLOSE all connections and terminate this process \n\n\
[SEND <Connection id> <message> - send message to host on the connectoin that is designated by the connection ID,\n[More than 100 characters won't send']\n\n";
char* display = 
"Author : ISSAC CHOI \n\
UBIT : issaccho\n\
UBMAIL: issaccho@buffalo.edu\n\
Current IP ADDR:";
//____________________________________________//
unsigned short int income_port;
char mode;
//_____________________________________________//

//For server
int serv_IP[MAX_CONNECT];
int num_connection;

//For client
int cli_IP[MAX_CLI_CONNECT];
int cli_con_max;

//______________________________________________//
struct sockaddr_in t_addr;

//buffer socket for binding
socklen_t sock_len = sizeof(t_addr);
// size of addr is all same so doesnt matter which one i choose
char buff[INET_ADDRSTRLEN];
//buffer for getIP()
char message [MAX_MSGSIZE+1];
//buffer for sending message
//Add 1 for null terminator;

//for get IP of local host at external eth
char* getIP() {
	int tmp_sfd;
	struct sockaddr_in temp;
	//make temp socket fd and sockaddr_in
	
	if((tmp_sfd = socket(AF_INET, SOCK_DGRAM,0)) == -1){
		perror("ERROR! CAN NOT GET IP! : ");
		exit(1);
	}
	//make new socket using UDP!
	
	bzero(&temp,sizeof(temp)); 
	//zeroing all member in temp
	
	temp.sin_family = AF_INET;
	temp.sin_port = htons(53);
	temp.sin_addr.s_addr=inet_addr("8.8.8.8");
	//set destin information: Google - 8.8.8.8 and UDP port 53 using IPv4
	
	if(connect(tmp_sfd, (const struct sockaddr*) &temp, sizeof(temp))==-1){
		perror("ERRROR!! CANNOT GET IP!! : ");
		exit(1);
	}
	// connect to destination
	// at this point, connected socket on google will have my information

	socklen_t size = sizeof(temp);
	if((getsockname(tmp_sfd, (struct sockaddr*)&temp, &size)) == -1){
		perror("ERR! :");
		exit(1);
	}
	//try to save that information to temp we created

	inet_ntop(AF_INET, &temp.sin_addr, buff, INET_ADDRSTRLEN);
	//put that in buff 
	
	close(tmp_sfd);
	//and close it 
	
	return buff;
	//return the IPaddress!
}

//to determine if given string is IPaddress
int isValidIP(char *ip){
	struct sockaddr_in t;
	int result = inet_pton(AF_INET, ip, &(t.sin_addr));
	return result != 0;
}
    
//parser method. mainly used by client.
int cli_parse(char **token){
		
	if(strcmp(token[0],"HELP")== 0 && numTok == 1){
		printf(cli_usage);		
	}else if(strcmp(token[0],"DISPLAY")== 0 && numTok == 1){
	
		printf(display);
		printf(getIP());
		printf("\nListening port :  %d\n",income_port);
		
	}else if(strcmp(token[0],"REGISTER")== 0 && numTok == 3){
	
		t_addr.sin_family = AF_INET;
		t_addr.sin_port   = htons(*token[2]);

		if(!isValidIP(token[1])){
		//if given address is not form of IP
			struct hostent *host;
			if((host=gethostbyname(token[1])) == NULL){
			//tries to get IP 
				herror("error on getting addr on register..");
			}
			t_addr.sin_addr = *((struct in_addr*)host->h_addr);	
		}else{
			t_addr.sin_addr.s_addr = inet_addr(token[1]); 
		}

		if(connect(cli_IP[0], (struct sockaddr*)&t_addr, sizeof(t_addr)) == -1){
		//tries to connect to server
			perror("REGIESTER CONNECTION");
		}
	}else if(strcmp(token[0],"CONNECT")== 0 && numTok == 3){
		
	}else if(strcmp(token[0],"LIST")== 0 && numTok == 1){
		
	}else if(strcmp(token[0],"TERMINATE")== 0 && numTok == 2){
		
	}else if(strcmp(token[0],"QUIT")== 0 && numTok == 1){
		printf("EXITING\n");
		return TRUE;
	}else if(strcmp(token[0],"SEND")== 0 && numTok >= 3){
		
	}else{
		printf(cli_usage);		
		printf("Please follow the format!\n\n");
	}
	return FALSE;
}
//for accetping client, used by server
int runServ(){
	fd_set readfds, tempfds;
	//two fd_set, one for original, and the other for tracking
	
	int maxfd = serv_IP[0];
	int fd_count;
	//maxfd for indexing fd_set
	//fd_count for tracking not resolved fd_set
	
	FD_ZERO(&readfds);
	//should be zero first to use select
	FD_SET(serv_IP[0],&readfds);
	//then mark server's listening port 
	printf("REACHED P1\n");
	while(num_connection < MAX_CONNECT){
		tempfds = readfds;
		fd_count = select(maxfd+1, &tempfds, NULL,NULL,NULL);
		printf("REACHED P2\n");

		if(FD_ISSET(serv_IP[0], &tempfds)){
		//Case when a client tries to connecto to the server
			if((serv_IP[num_connection] = accept(serv_IP[0], (struct sockaddr*) &t_addr, &sock_len))== -1){
				perror("ERROR cant accept :");
				exit(1);
			}
printf("REACHED P3\n");

			FD_SET(serv_IP[num_connection++],&readfds);
			//put new client into the readfds and increase num_connection by 1
			send(serv_IP[num_connection-1],"WELCOME",strlen("WELCOME"),0);
			fd_count--;
			continue;
		}
		if(fd_count > 0){ 
		//if it is from other client,
			int i;
			for(i=0; i < maxfd; i++){
				if(FD_ISSET(i,&tempfds)){

				}	
			}	
		}
	}	
	if(num_connection >=MAX_CONNECT) printf("MAX CONNECTION REACHED. Some one has to quit from the listi\n The server is collapsing..pshhhhhhh\n\n");

	return TRUE;
}
//initializing socket for server or client 
void init(){
	switch(mode){		
		case 's':
			if((serv_IP[0] = socket(AF_INET, SOCK_STREAM,0)) == -1){
				perror("ERROR! CREATING SOCK :");
				exit(1);
			}
			//creating server_socket with IP4!

			bzero(&t_addr, sizeof(t_addr));
			//initializing socket structure for unsigned char sin_zero[8]
			t_addr.sin_family = AF_INET;
			//using IPv4
			t_addr.sin_port = htons(income_port);
			//using port from the argv for listening
			t_addr.sin_addr.s_addr = inet_addr(getIP());
			//set listening IP to local IP

			if(bind(serv_IP[0], (struct sockaddr*)&t_addr, sizeof(t_addr)) == -1){
				perror("Serv- BIND ERR:");
				exit(1);
			}
			//then bind it, if it fails, print out the error
			if(listen(serv_IP[0], BACKLOG) == -1){
				perror("LISTEN ERR:");
				exit(1);
			}
			num_connection = 1;
			break;

		case 'c':
		//same for client except, it only needs to create socket
			if((cli_IP[0] = socket(AF_INET, SOCK_STREAM, 0))==-1){
				perror("ERROR! SOCKCREATE:");
				exit(1);
			}
			bzero(&t_addr, sizeof(t_addr));
			t_addr.sin_family = AF_INET;
			t_addr.sin_port = htons(income_port);
			t_addr.sin_addr.s_addr = inet_addr(getIP());
			if(bind(cli_IP[0], (struct sockaddr*)&t_addr, sizeof(t_addr)) == -1){
				perror("Cli- BIND ERR:");
				exit(1);
			}
			cli_con_max = 1;	
			break;
	}
}
