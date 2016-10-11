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
char* display = 
"Author : ISSAC CHOI \n\
UBIT : issaccho\n\
UBMAIL: issaccho@buffalo.edu\n\
Current IP ADDR:";
//____________________________________________//
//________________From MAIN__________________//
unsigned int numTok;
unsigned short int income_port;
char mode;
//_____________________________________________//
//_________________For server_________________//
sock serv_IP[MAX_CONNECT];
int num_connection;
//____________________________________________//
//_________________For client________________//
sock cli_IP[MAX_CLI_CONNECT];
int cli_con_max;
//______________________________________________//
//_______________GENERAL PURPOSE________________//
struct sockaddr_in t_addr;
//buff socket for general use
socklen_t sock_len = sizeof(t_addr);
// size of addr is all same so doesnt matter which one i choose
char getip[INET_ADDRSTRLEN];
//buffer for getLOIP()
//for geti IP of local host at external eth
char gethost[50];
//for gethost buffer for saving hostname
char list[MAX_LIST];
//for LIST method();
char *command;
char **tokens;
//_________________________________________________//
int isValidIP(char *ip){
	struct sockaddr_in t;
	int result = inet_pton(AF_INET, ip, &(t.sin_addr));
	return result != 0;
}
//to determine if given string is IPaddress

int findEmptySock(){
	int i;
	if(mode == 's'){
		for(i = 0; i < MAX_CONNECT; i++){
			if(serv_IP[i].sock ==0) return i;
		}
	}else{
		for(i = 0; i < MAX_CLI_CONNECT; i++){
			if(cli_IP[i].sock ==0) return i;
		}
	}
} 
//Return first index of serv_IP that has sock value of 0

int findMaxFd(int cur_sock){
	int i;
	int maxfd=0;
	for(i = 0; i < MAX_CONNECT; i++){
		if(serv_IP[i].sock > maxfd && maxfd < cur_sock) maxfd = serv_IP[i].sock;
	}
	return maxfd;
}
// When client quit, it needs to check if quitting client has max socket number.
// if it holds, switch to second highest socket number.

char* getHost(struct sockaddr_in addr){
	t_addr = addr;
	getnameinfo((struct sockaddr*)&t_addr, sock_len, gethost, sizeof(gethost),NULL,0,0);
	return gethost;
}

char* getIP(char* hostname){
	struct hostent *h;
	if((h = gethostbyname(hostname)) ==NULL){
		herror("Cannot resolve IP from hostname..");
		return NULL;
	}
	memcpy(&t_addr.sin_addr, h->h_addr_list[0], h->h_length);
	inet_ntop(AF_INET, &t_addr.sin_addr, getip, INET_ADDRSTRLEN);
	return getip;
}
// will return hostname from IP address and sve to gethost
char* getLOIP() {
 	sock temp;
	bzero(&temp,sizeof(temp)); 
	//zeroing all member in temp
	if((temp.sock = socket(AF_INET, SOCK_DGRAM,0)) == -1){
		perror("ERROR! CAN NOT GET IP! : ");
		exit(1);
	}
	//make new socket using UDP!
	temp.sock_info.sin_family = AF_INET;
	temp.sock_info.sin_port = htons(53);
	temp.sock_info.sin_addr.s_addr=inet_addr("8.8.8.8");
	//set destin information: Google - 8.8.8.8 and UDP port 53 using IPv4	
	if((connect(temp.sock, (struct sockaddr*)&temp.sock_info, sock_len)) == -1) perror("CANNOT GET MY IP:");
	//connect to google server via udp
	if((getsockname(temp.sock, (struct sockaddr*)&temp.sock_info,&sock_len)) == -1) perror("CanNOT GET SOCK INFO");
	//retrieve my socket information on google's side and save to
	inet_ntop(AF_INET, &temp.sock_info.sin_addr, getip, INET_ADDRSTRLEN);
	//and resolve network address to string IP format
	return getip;
}
//get public ip using UDP connection 
void LIST(){
	sprintf(list, "id:	Hostnamed				IP		port\n");
	//init LIST format
	int count;
	if(mode == 's'){
		for(count=0; count < MAX_CONNECT; count++){
			if(serv_IP[count].sock != 0){
				sprintf(list+strlen(list),"%d:	%s		%s	%d\n"\
				,count, getHost(serv_IP[count].sock_info),inet_ntoa(serv_IP[count].sock_info.sin_addr), ntohs(serv_IP[count].sock_info.sin_port));
			}
		}
	}else{
		for(count=0; count < MAX_CLI_CONNECT; count++){
			if(cli_IP[count].sock != 0){
				sprintf(list+strlen(list),"%d:	%s		%s	%d\n"\
				,count, getHost(cli_IP[count].sock_info),inet_ntoa(cli_IP[count].sock_info.sin_addr), ntohs(cli_IP[count].sock_info.sin_port));
			}
		}
	}
	/*It will make a list of connected hosts to caller
 	 *If the server calls this, it will make list based on server's list
	 *But if a client calls List, it will build list based on client's connected list*/
}
int runServ(){
	fd_set readfds, tempfds;
	//for multiplexing of inputs from keyboard and sockets,
	int maxfd = serv_IP[0].sock;
	//set current range of selection is socket# of server
	int fd_count;
	//will used to count how many inputs are coming
	FD_ZERO(&readfds);
	//init fd_set
	FD_SET(serv_IP[0].sock,&readfds);
	FD_SET(0,&readfds);	
	//mark stdin and socket# of server
	
	while(TRUE){
		fprintf(stderr,"PA1> ");
		tempfds = readfds;
		//renew fd_set
//		fd_count = select(maxfd+1, &tempfds,NULL,NULL,NULL);
		//wait until something is coming...

		for(fd_count = select(maxfd+1,&tempfds, NULL,NULL,NULL); fd_count > 0; fd_count--){
			if(FD_ISSET(serv_IP[0].sock,&tempfds)){
			//if a client tries to connect to sever
				int freespot = findEmptySock();
				printf("%d\n\n",freespot);
				//find empty position on server list
				//enable reusing spots in where client quitted.
				if((serv_IP[freespot].sock = accept(serv_IP[0].sock,(struct sockaddr*) &serv_IP[freespot].sock_info, &sock_len))==-1) perror("CANNOT ACCEPT: ");
				//accept client and all information about client is saved in serv_IP
				char action[10];
				int terminator = recv(serv_IP[freespot].sock, action, 10, 0);
				action[terminator]='\0';
				//if a client tries to connect..
				if(strcmp(action,"REGISTER")!=0){
					send(serv_IP[freespot].sock, "s", strlen("s"),0);
					//send client if I am the server!
					if(serv_IP[freespot].sock > maxfd) maxfd = serv_IP[freespot].sock;
					//renew max range of fdset if connected socket number is greater than current maxfd
					FD_SET(serv_IP[freespot].sock,&readfds);
					//mark client on readfds
					LIST();
					//renew the list
					printf(list);
					//and print it to the server too
					num_connection++;
				}else{
					close(serv_IP[freespot].sock);
					serv_IP[freespot].sock =0;
				}
			}else if(FD_ISSET(0,&tempfds)){
			//if it is from keyboard,
				command = readLine();
				tokens = parseTok(command);
				//parse the reader input and tokenize it.
				if(tokens[0] == '\0'){
					continue;
				}else if(strcmp(tokens[0], "HELP") ==0 && numTok == 1){
					printf(cli_usage);
				}else if(strcmp(tokens[0],"DISPLAY")==0 && numTok == 1){
					printf(display);
					printf(" %s \n",getLOIP());
					printf("Income port : %d", income_port);
				}else if(strcmp(tokens[0], "CONNECT") ==0 && numTok ==3){

				}else if(strcmp(tokens[0], "LIST") ==0 && numTok ==1){
					LIST();
					printf(list);
				}else if(strcmp(tokens[0], "TERMINATE") ==0 && numTok ==2){

				}else if(strcmp(tokens[0], "QUIT") == 0 && numTok ==1){

				}else if(strcmp(tokens[0], "SEND") == 0 && numTok > 3 && numTok < 40){
					printf("you cannot send from server\n")	;
				}else{
					printf(cli_usage);
				}
			}else{
			//message from other clients
			}
		}
			//processing keyboard input from server itself..
	}			
}

int runCli(){
	fd_set readfds, tempfds;
	int maxfd = cli_IP[0].sock;
	int fd_count;
	int REGISTERED = FALSE;
	FD_ZERO(&readfds);
	FD_SET(cli_IP[0].sock,&readfds);
	FD_SET(0,&readfds);	
	//initial setting of client just same as the server
	while(TRUE){
		fprintf(stderr,"PA1> ");
		//init number of TOken and print out the shell
		tempfds = readfds;
		//renew the fdset
		for(fd_count = select(maxfd+1,&tempfds, NULL,NULL,NULL); fd_count > 0; fd_count--){
		//wait until there is an input and starts to work at coming inputs.
			if(FD_ISSET(cli_IP[0].sock,&tempfds)){
			//if other clients connecto this client.	
				if(cli_con_max < MAX_CLI_CONNECT){
				//if client does not have its max connections with other peer,
					int freespot = findEmptySock();
					//search for the empty socket
					if((cli_IP[freespot].sock = accept(cli_IP[0].sock,(struct sockaddr*) &cli_IP[freespot].sock_info, &sock_len))==-1) perror("CANNOT ACCEPT: ");
					//and accept it with listening socket, and save info of the connecting peer
					char action[3];
					int terminator = recv(cli_IP[freespot].sock, action, strlen(action), 0);
					action[terminator]='\0';
					//if a client tries to connect..
					if(action=="R"){
						send(cli_IP[freespot].sock, "c",sizeof("c"),0);
						close(cli_IP[freespot].sock);
						cli_IP[freespot].sock =0;
					}else{
						if(cli_IP[freespot].sock > maxfd) maxfd = cli_IP[freespot].sock;
						//if socketnubmer of newly connected socket is larger than original maxfd, update its value.
						FD_SET(cli_IP[freespot].sock,&readfds);
						//and add socket to the readfds.
						LIST();
						//renew the list
						printf("NEW Friend!\n");
						printf(list);
						//and print out the list.
						cli_con_max++;
						//increase number of connection by 1
					}
				}else{
					printf("IP list is full..Ignore all incoming\n");
					continue;
				}
			}else if(FD_ISSET(0,&tempfds)){
			//if it is from keyboard,
				command = readLine();
				tokens = parseTok(command);
				//parse the reader input and tokenize it.
				if(tokens[0] == '\0'){
					continue;
				}else if(strcmp(tokens[0], "HELP") ==0 && numTok == 1){
					printf(cli_usage);
				}else if(strcmp(tokens[0],"DISPLAY")==0 && numTok == 1){
					printf(display);
					printf(" %s \n",getLOIP());
					printf("Income port : %d", income_port);
				}else if(strcmp(tokens[0], "REGISTER") == 0 && numTok ==3){
				//register to the server!
					if((cli_IP[1].sock = socket(AF_INET, SOCK_STREAM,0))!=-1){
					//made new socket to connect to the server 
						cli_IP[1].sock_info.sin_family = AF_INET;
						//use ipv4
						cli_IP[1].sock_info.sin_port = htons(atoi(tokens[2]));
						//set typed port number
					
						if(isValidIP(tokens[1])) cli_IP[1].sock_info.sin_addr.s_addr = inet_addr(tokens[1]);
						else cli_IP[1].sock_info.sin_addr.s_addr =inet_addr(getIP(tokens[1]));
						//test if given string is ipforman or hostname.
						//if it is hostname, resolve it to IP string and set its address,
						//or just put it
						if((connect(cli_IP[1].sock,(struct sockaddr*) & cli_IP[1].sock_info, sock_len))==-1) perror("Cannot Register:");
						//connect to the server
						
						send(cli_IP[1].sock,"R",sizeof("R"),0);
						//let them know that i am trying to register..

						char ans[3];
						int terminator = recv(cli_IP[1].sock, ans, strlen(ans),0);
						ans[terminator] = '\0';
						//receive from server that it is really the server
						if(ans[0]=='s'){	
						//if server everything is good!
							FD_SET(cli_IP[1].sock,&readfds);
							maxfd = cli_IP[0].sock > cli_IP[1].sock ? cli_IP[0].sock : cli_IP[1].sock;
							cli_con_max++;
							REGISTERED = TRUE;
							LIST();
							printf(list);
						}else{
							close(cli_IP[1].sock);
							cli_IP[1].sock = 0;
							printf("you are not connecting to the server..");
						}
					}else{
					}
				}else if(strcmp(tokens[0], "CONNECT") ==0 && numTok ==3){

				}else if(strcmp(tokens[0], "LIST") ==0 && numTok ==1){
					LIST();
					printf(list);
				}else if(strcmp(tokens[0], "TERMINATE") ==0 && numTok ==2){

				}else if(strcmp(tokens[0], "QUIT") == 0 && numTok ==1){

				}else if(strcmp(tokens[0], "SEND") == 0 && numTok > 3 && numTok < 40){
				
				}else{
					printf(cli_usage);
				}
			}else{
			//message from other clients
			}
		}
			//processing keyboard input from server itself..
	}			
}


void init(){	
	switch(mode){
	case 's':
		bzero(&serv_IP, sizeof(serv_IP));
		//init server array with zeros

		if((serv_IP[0].sock = socket(AF_INET, SOCK_STREAM, 0)) ==-1){
			perror("Error! Sockcreate - serv:");
			exit(1);
		}
		serv_IP[0].sock_info.sin_family = AF_INET;
		//using tcp 
		serv_IP[0].sock_info.sin_port = htons(income_port);
		//using port from the argv for listening
		serv_IP[0].sock_info.sin_addr.s_addr = inet_addr(getLOIP());
		//set listening IP to local IP
		if(bind(serv_IP[0].sock, (struct sockaddr*)&serv_IP[0].sock_info, sizeof(serv_IP[0])) == -1){
			perror("Serv- BIND ERR:");
			exit(1);
		}
		//then bind it, if it fails, print out the error
		if(listen(serv_IP[0].sock, BACKLOG) == -1){
			perror("LISTEN ERR:");
			exit(1);
		}
		num_connection = 1;
		break;

	case 'c':
		//same for client except, it only needs to create socket
		bzero(&cli_IP, sizeof(cli_IP));
		if((cli_IP[0].sock = socket(AF_INET, SOCK_STREAM, 0))==-1){
			perror("ERROR! SOCKCREATE-cli :");
			exit(1);
		}
		cli_IP[0].sock_info.sin_family = AF_INET;
		cli_IP[0].sock_info.sin_port = htons(income_port);
		cli_IP[0].sock_info.sin_addr.s_addr = inet_addr(getLOIP());
		                       cli_con_max++;
           	if((bind(cli_IP[0].sock, (struct sockaddr*)&cli_IP[0].sock_info, sock_len)) == -1){
			perror("Cli- BIND ERR:");
			exit(1);
		}
		
		if(listen(cli_IP[0].sock, BACKLOG) == -1){
			perror("Client listen");
			exit(1);
		}
		LIST();
		printf(list);
		cli_con_max = 1;
		break;
	}
}
//initializing socket for server or client 
