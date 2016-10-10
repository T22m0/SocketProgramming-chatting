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
char* display = 
"Author : ISSAC CHOI \n\
UBIT : issaccho\n\
UBMAIL: issaccho@buffalo.edu\n\
Current IP ADDR:";
//____________________________________________//
//________________From MAIN__________________//
unsigned short int income_port;
char mode;
//_____________________________________________//
//_________________For server_________________//
server serv_IP[MAX_CONNECT];
int num_connection;
//____________________________________________//
//_________________For client________________//
int cli_IP[MAX_CLI_CONNECT];
int cli_con_max;
//______________________________________________//
//_______________GENERAL PURPOSE________________//
struct sockaddr_in t_addr;
//buff socket for general use
socklen_t sock_len = sizeof(t_addr);
// size of addr is all same so doesnt matter which one i choose
char getip[INET_ADDRSTRLEN];
//buffer for getIP()
//for geti IP of local host at external eth
char gethost[50];
//for gethost buffer for saving hostname
char list[MAX_LIST];
//for LIST method();

int isValidIP(char *ip){
	struct sockaddr_in t;
	int result = inet_pton(AF_INET, ip, &(t.sin_addr));
	return result != 0;
}
//to determine if given string is IPaddress

int findEmptySock(){
	int i;
	for(i = 0; i < MAX_CONNECT; i++){
		if(serv_IP[i].sock ==0) return i;
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
// will return hostname from IP address and sve to gethost
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
	
???LINES MISSING
???LINES MISSING
			serv_IP[0].sock_info.sin_port = htons(income_port);
			//using port from the argv for listening
			serv_IP[0].sock_info.sin_addr.s_addr = inet_addr(getIP());
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
			if((cli_IP[0] = socket(AF_INET, SOCK_STREAM, 0))==-1){
				perror("ERROR! SOCKCREATE:");
				exit(1);
			}
			bzero(&t_addr, sizeof(t_addr));
			t_addr.sin_family = AF_INET;
			t_addr.sin_port = htons(income_port);
			t_addr.sin_addr.s_addr = inet_addr(getIP());
		
			int temp;
			if((temp=bind(cli_IP[0], (struct sockaddr*)&t_addr, sizeof(t_addr))) == -1){
				perror("Cli- BIND ERR:");
				exit(1);
			}
			cli_con_max = 1;	
			break;
	}
}
//initializing socket for server or client 
