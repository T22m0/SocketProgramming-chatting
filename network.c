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
char* serv_usage =  
"USAGE:\n\n\
[HELP] - print out this instruction \n\n\
[DISPLAY] - display information\n[name, UBIT name, UB e-mail, IP address of the process, port this process is listening.\n\n\
[LIST] - Display numbered list of all the connections this process if part of\n\n\
[QUIT] - CLOSE all connections and terminate this process \n\n";
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
char r_command[MAX_COMMAND];
//for server, used for receive msg from clients
//____________________________________________//
//_________________For client________________//
sock cli_IP[MAX_CLI_CONNECT];
int cli_con_max;
//______________________________________________//
//_______________GENERAL PURPOSE________________//
fd_set readfds, tempfds;
//for multiplexing of inputs from keyboard and sockets,
int fd_count;
//will used to count how many inputs are coming
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
char message[MAX_MSG];
//for sending and receiving message
char *command;
char **tokens;
//for interpretate user inputs
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
int findMaxFd(int cur_sock,int maxfd){
	int i;
	if(cur_sock < maxfd) return maxfd;
	else if(cur_sock == maxfd){
		int temp =0;
		if(mode == 's'){
			for(i = 0; i < MAX_CONNECT; i++){
				if(serv_IP[i].sock > temp && temp < cur_sock) temp = serv_IP[i].sock;
			}
		}else{
			for(i = 0; i< MAX_CLI_CONNECT; i++){
				if(cli_IP[i].sock > temp && temp < cur_sock) temp = cli_IP[i].sock;
			}
		}
		return temp;
	}
}
// When client quit, it needs to check if quitting client has max socket number.
// if it holds, switch to second highest socket number.
char* getHost(struct sockaddr_in addr){
	t_addr = addr;
	getnameinfo((struct sockaddr*)&t_addr, sock_len, gethost, sizeof(gethost),NULL,0,0);
	return gethost;
}
//get host name based on ipaddress..
char* getIP(char* hostname){
	struct hostent *h;
	if((h = gethostbyname(hostname)) ==NULL){
		herror("Cannot resolve IP from hostname..");
		sprintf(getip,"NA");
		return getip;
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
			if(serv_IP[count].sock != 0 && count == 0){
				sprintf(list+strlen(list),"%d:	%s		%s	%d\n"\
				,count+1, getHost(serv_IP[count].sock_info),inet_ntoa(serv_IP[count].sock_info.sin_addr), ntohs(serv_IP[count].sock_info.sin_port));
			}else if(serv_IP[count].sock != 0){
				sprintf(list+strlen(list),"%d:	%s		%s	%d\n"\
				,count+1, getHost(serv_IP[count].sock_info),inet_ntoa(serv_IP[count].sock_info.sin_addr), ntohs(serv_IP[count].sock_info.sin_port)-1);
			}
		}
	}else{
		for(count=0; count < MAX_CLI_CONNECT; count++){
			if(cli_IP[count].sock != 0){
				sprintf(list+strlen(list),"%d:	%s		%s	%d\n"\
				,count+1, getHost(cli_IP[count].sock_info),inet_ntoa(cli_IP[count].sock_info.sin_addr), ntohs(cli_IP[count].sock_info.sin_port));
			}
		}
	}
	printf(list);
	/*It will make a list of connected hosts to caller
 	 *If the server calls this, it will make list based on server's list
	 *But if a client calls List, it will build list based on client's connected list*/
}
void broadcast_msg(char* msg){
	int i;
	for(i = 1; i <MAX_CONNECT; i++){
		if(serv_IP[i].sock != 0){
			send(serv_IP[i].sock, msg, strlen(msg),0);
		}
	}	
}
//used by server. broadcast message to all existing client

int close_all(){
	int i;
	if(mode =='s'){
		for(i = 1; i < MAX_CONNECT; i++) if(serv_IP[i].sock !=0) close(serv_IP[i].sock);
		return 1;
	}else if(mode =='c'){
		for(i = 1; i < MAX_CLI_CONNECT; i++) if(cli_IP[i].sock!=0) close(cli_IP[i].sock);
		return 1;
	}
		return -1;
}
//used for closing all the existing connection.
int runServ(){
	int maxfd = serv_IP[0].sock;
	//set current range of selection is socket# of server
	FD_SET(serv_IP[0].sock,&readfds);
	FD_SET(0,&readfds);	
	//mark stdin and socket# of server
	
	while(TRUE){
		fprintf(stderr,"PA1> ");
		tempfds = readfds;
		//renew fd_set
		fd_count = select(maxfd+1,&tempfds, NULL,NULL,NULL);
		//wait until something happens..
		
		while(fd_count-- > 0){
		//decreade fd_count because we know something will be done in here.

			if(FD_ISSET(serv_IP[0].sock,&tempfds)){
			//if a client tries to connect to sever
			   if(num_connection <= MAX_CONNECT){
				int freespot = findEmptySock();
				//find empty position on server list
				//enable reusing spots in where client quitted.
				if((serv_IP[freespot].sock = accept(serv_IP[0].sock,(struct sockaddr*) &serv_IP[freespot].sock_info, &sock_len))==-1) perror("CANNOT ACCEPT: ");
				//accept client and all information about client is saved in serv_IP
				char action[3];
				int terminator = recv(serv_IP[freespot].sock, action, 3, 0);
				action[terminator]='\0';
				//if a client tries to connect..
				if(strcmp(action,"R")==0){
					send(serv_IP[freespot].sock, "s", strlen("s"),0);
					//send client if I am the server!
					if(serv_IP[freespot].sock > maxfd) maxfd = serv_IP[freespot].sock;
					//renew max range of fdset if connected socket number is greater than current maxfd
					FD_SET(serv_IP[freespot].sock,&readfds);
					//mark client on readfds
					LIST();
					broadcast_msg(list);
					//broadcast all client!
					num_connection++;
				}else{
				//what..? not 'R' for registering..? someone's hacking..
					close(serv_IP[freespot].sock);
					serv_IP[freespot].sock =0;
					//close the connection and wipeout the socket number
					continue;
				}
			   }else{
				printf("User OverFlow.. Ignoring all incoming!");
				continue;
			   }
			}else if(FD_ISSET(0,&tempfds)){
			//if it is from keyboard,
				command = readLine();
				tokens = parseTok(command);
				//parse the reader input and tokenize it.
				if(tokens[0] == '\0'){
					continue;
				//empty input.. ignore
				}else if(strcmp(tokens[0], "HELP") ==0 && numTok == 1){
					printf(serv_usage);
				//print out server usage
				}else if(strcmp(tokens[0],"DISPLAY")==0 && numTok == 1){
					printf(display);
					printf(" %s \n",getLOIP());
					printf("Income port : %d", income_port);
				//print current machine info
				}else if(strcmp(tokens[0], "LIST") ==0 && numTok ==1){
					LIST();
				//LIST current connection
				}else if(strcmp(tokens[0], "QUIT") == 0 && numTok ==1){
					broadcast_msg("THE server is going off...\n");
					if(close_all()) printf("CLOSED ALL CONNECTION\n");
					exit(1);						
				}else{
					printf(serv_usage);
				}
			}else{
			//message from other clients
				int t;
				for(t =1; t<MAX_CONNECT; t++){
				//from first client index of serv_IP
					if(FD_ISSET(serv_IP[t].sock,&tempfds)){			
					//if the socket is triggered..
						int terminator = recv(serv_IP[t].sock, r_command, MAX_COMMAND, 0);
						//try to read from te client
						if(terminator ==0){
						//but if figured out the client is left for nothing..
							printf("CLIENT LEFT!!\n");
							maxfd =	findMaxFd(serv_IP[t].sock,maxfd);
							//find next highest socket and assign to maxfd							
							close(serv_IP[t].sock);
							//finish the socket
							FD_CLR(serv_IP[t].sock,&readfds);
							//unmark from readfds
							serv_IP[t].sock = 0;
							//init socket number.
							LIST();
							//update list and
							broadcast_msg(list);
							//broadcast to the clients
							num_connection--;
						}else{
						//if there is incoming..
							r_command[terminator]='\0';
							tokens = parseTok(r_command);
							//parse token.
							if(strcmp(tokens[0], "LIST")==0 && numTok ==1){
								send(serv_IP[t].sock,list,MAX_LIST,0);
								//send(serv_IP[t].sock, list, MAX_LIST,0);
							}else if(strcmp(tokens[0], "CONNECT")==0 && numTok ==3){
								int ip,i,p;
								//ip if it is form of IP form, i for index, p for temp port
								char* str;
								//for saving temp ip address
								if(!(ip=isValidIP(tokens[1]))) getIP(tokens[1]);
								for(i =1; i< MAX_CONNECT; i++){
								   if(serv_IP[i].sock!=0){
								      str = inet_ntoa(serv_IP[i].sock_info.sin_addr);
								      p = ntohs(serv_IP[i].sock_info.sin_port);
								      //parse information from the server IP addr, and port
									if(ip){
									//if it is ipForm, compare with tokens[1] else, getip
									   if(strcmp(inet_ntoa(serv_IP[i].sock_info.sin_addr),tokens[1]) ==0 &&\
									     (ntohs(serv_IP[i].sock_info.sin_port)-1==atoi(tokens[2]))){
									     //if IP matches, and port number matches
									     //*note that listening port is 1 less than port to server
										send(serv_IP[t].sock,"Y",strlen("Y"),0);
									        //send YES there is!
										break;
									   }  
									}else{
									    if(strcmp(inet_ntoa(serv_IP[i].sock_info.sin_addr),getip) ==0 &&\
									      (ntohs(serv_IP[i].sock_info.sin_port)-1==atoi(tokens[2]))){
										send(serv_IP[t].sock,"Y",strlen("Y"),0);
									        //send YES there is!
										break;
									    }
									}
								   }
								}if(i==MAX_CONNECT) send(serv_IP[t].sock, "N",strlen("N"),0);
														
							}else if(strcmp(tokens[0], "QUIT") == 0 && numTok ==1){					
								send(serv_IP[t].sock,"Q",strlen("0"),0);
								//you are approved to leave my list!
								maxfd = findMaxFd(serv_IP[t].sock,maxfd);
								//find next highest maxfd
								close(serv_IP[t].sock);
								//close with the client
								FD_CLR(serv_IP[t].sock,&readfds);
								//unmark it
								serv_IP[t].sock =0;
								//init socket
								LIST();
								broadcast_msg(list);
								//update list and broadcast it
								num_connection--;
							}
						}
					}
					if(fd_count-- < 0) break;
					//need to check so that another epoch of select could excute
				}
			}
		}
			//processing keyboard input from server itself..
	}			
}

int runCli(){
	int maxfd = cli_IP[0].sock;
	int REGISTERED = FALSE;
	FD_SET(cli_IP[0].sock,&readfds);
	FD_SET(0,&readfds);	
	//initial setting of client just same as the server
	while(TRUE){
		fprintf(stderr,"PA1> ");
		//init number of TOken and print out the shell
		tempfds = readfds;
		//renew the fdset	
		fd_count = select(maxfd+1,&tempfds, NULL,NULL,NULL);
		while(fd_count-- > 0){
		//wait until there is an input and starts to work at coming inputs.
			if(FD_ISSET(cli_IP[0].sock,&tempfds)){
			//if other clients connecto this client.	
				if(cli_con_max <= MAX_CLI_CONNECT){
				//if client does not have its max connections with other peer,
					int freespot = findEmptySock();
					//search for the empty socket
					if((cli_IP[freespot].sock = accept(cli_IP[0].sock,(struct sockaddr*) &cli_IP[freespot].sock_info, &sock_len))==-1) perror("CANNOT ACCEPT: ");		
					char action[3];
					int terminator = recv(cli_IP[freespot].sock, action, strlen(action), 0);
					action[terminator]='\0';
					//if a client tries to connect..
					printf("%s IS THE MESSAGE\n",action);
					//and accept it with listening socket, and save info of the connecting peer
					if(strcmp(action,"R")==0){
					//if received token if "R", then it is from client who is connecting
						send(cli_IP[freespot].sock, "c",sizeof("c"),0);
						//I am the client
						close(cli_IP[freespot].sock);
						//I am shutting your connection
						cli_IP[freespot].sock =0;
						//go away
						continue;
					}else if(strcmp(action,"C")==0){
					//it is COnnecting..
						send(cli_IP[freespot].sock,"Y",strlen("Y"),0);
						if(cli_IP[freespot].sock > maxfd) maxfd = cli_IP[freespot].sock;
						//if socketnubmer of newly connected socket is larger than original maxfd, update its value.
						FD_SET(cli_IP[freespot].sock,&readfds);
						//and add socket to the readfds.
						LIST();
						//renew the list
						printf("NEW Friend!\n");
						//and print out the list.
						cli_con_max++;
						//increase number of connection by 1
					}
				}else{
					printf("IP list is full..say NO! to all incoming\n");
					int temp = accept(cli_IP[0].sock, (struct sockaddr*) &t_addr, &sock_len);
					//temporary accept its call
					send(temp, "N",strlen("N"),0);
					//then... NO!
					close(temp);
					//I am leaving, not a word to talk to
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
			           if(REGISTERED==FALSE){
					cli_IP[1].sock_info.sin_port = htons(atoi(tokens[2]));
					//set typed port number
					if(isValidIP(tokens[1])) cli_IP[1].sock_info.sin_addr.s_addr = inet_addr(tokens[1]);
					//if given domain is in IPaddr form, just put that in addr section
					else{ 
					//or if it is hostname
						if(strcmp((getIP(tokens[1])),"NA")==0){
							printf("You put wrong hostname..\n");
							continue;
						}
						//get ip with given host name and if cannot resolve.. print error and continue..
						else cli_IP[1].sock_info.sin_addr.s_addr =inet_addr(getip);
						//or assign ipaddress.
					}					
					if((connect(cli_IP[1].sock,(struct sockaddr*) & cli_IP[1].sock_info, sock_len))==-1){
					//pass server binded socket number
						perror("Cannot Register, Check the domain or port");
						continue;
					}
					//connect to the server
					
					send(cli_IP[1].sock,"R",sizeof("R"),0);
					//let them know that i am trying to register..
					char ans[3];
					int terminator = recv(cli_IP[1].sock, ans, strlen(ans),0);
					ans[terminator] = '\0';
					//receive from server that it is really the server
					if(strcmp(ans,"s")==0){	
						//if server everything is good!
						FD_SET(cli_IP[1].sock,&readfds);
						maxfd = cli_IP[0].sock > cli_IP[1].sock ? cli_IP[0].sock : cli_IP[1].sock;
						cli_con_max++;
						REGISTERED = TRUE;	
					}else{
						close(cli_IP[1].sock);
						cli_IP[1].sock = 0;
						printf("you are not connecting to the server..\n");
					}
				   }else{
					printf("YOU ALREADY REGISTERED TO SERVER\n");
					continue;
				   }
				}else if(strcmp(tokens[0], "CONNECT") ==0 && numTok ==3){
					if(REGISTERED==TRUE){
					//if registered
						if(cli_con_max <= MAX_CLI_CONNECT){
						//check if I have not reached max connection...
							sprintf(r_command,"%s %s %s",tokens[0],tokens[1],tokens[2]);
							//restore original command to send to server							
							send(cli_IP[1].sock,r_command,strlen(r_command),0);
							//if not ask server to verify the address
							int terminator = recv(cli_IP[1].sock,message,MAX_MSG,0);
							message[terminator]='\0';
							printf("GOT MESSAGE FROM SRVER : %s\n",message);
							//receive from server
							if(strcmp(message,"Y")==0){
							//if it is in server and server says YES!
								int freespot = findEmptySock();
								//find free spot
								if((cli_IP[freespot].sock=socket(AF_INET, SOCK_STREAM,0))==-1){
									perror("Connect socket ERR:");
									continue;
								}	
								//create sockets in free spot
								cli_IP[freespot].sock_info.sin_family = AF_INET; 
								cli_IP[freespot].sock_info.sin_port = htons(atoi(tokens[2]));
								if(isValidIP(tokens[1])) cli_IP[freespot].sock_info.sin_addr.s_addr = inet_addr(tokens[1]);
								else{	
									getIP(tokens[1]);
									cli_IP[freespot].sock_info.sin_addr.s_addr = inet_addr(getip);
								}
								if((connect(cli_IP[freespot].sock,(struct sockaddr*) & cli_IP[freespot].sock_info, sock_len))==-1){
								//try to connect to peer on given info
									perror("Cannot Connnect");
									continue;
								}else{
								//if can connect
									send(cli_IP[freespot].sock, "C",strlen("C"),0);
									terminator = recv(cli_IP[freespot].sock, message,MAX_MSG,0);
									message[terminator]='\0';
									printf("GOT MEESSAGE FROM CLIENT : %s\n",message);
									//receive message from the client if it also accepts
									if(strcmp(message,"Y")==0){
									//if yes
										LIST();
										FD_SET(cli_IP[freespot].sock,&readfds);
										if(maxfd < cli_IP[freespot].sock) maxfd = cli_IP[freespot].sock;
										cli_con_max++;
										//update info
									}else if(strcmp(message,"N")){
									//if No
										close(cli_IP[freespot].sock);
										cli_IP[freespot].sock =0;
										//close the connection
										printf("The other client reached max connection\n");	
									}
								}
							}else if(strcmp(message,"N")==0){
								if(strcmp(tokens[1],"127.0.0.1")==0){
									printf("Cannot Connect to yourself..i\n");
								}else{
									printf("Given IP is not in the Serverlist\n");
								}
							}
						}else{
							printf("Your Connection LIst is full \n");
						}
					}else{
						printf("YOU SHOULD REGISTER YOURSELF TO SERVER BEFORE CONNECT..\n");
						continue;
					}
				}else if(strcmp(tokens[0], "LIST") ==0 && numTok ==1){
					if(REGISTERED==TRUE){
						printf("__________CONNECTED CLIENT IP LIST_______\n");
						LIST();
						printf("__________SERVER CLIENT IP LIST__________\n");
						send(cli_IP[1].sock, "LIST", strlen("LIST"),0);
						int terminator = recv(cli_IP[1].sock, list, MAX_LIST,0);
						if(terminator) list[terminator] = '\0';
						printf(list);
					}else{
						printf("YOU SHOULD REGISTER TO GET FRIEND LIST!\n");
						continue;
					}
				}else if(strcmp(tokens[0], "TERMINATE") ==0 && numTok ==2){
					
				}else if(strcmp(tokens[0], "QUIT") == 0 && numTok ==1){
					if(REGISTERED==TRUE){
						send(cli_IP[1].sock,"QUIT", strlen("QUIT"), 0);
						//notifying server that the client is quitting
						int terminator = recv(cli_IP[1].sock, message, MAX_MSG,0);
						//receive from server if quitting is confirmed
						if(terminator) message[terminator] = '\0';
						if(strcmp(message, "Q") == 0){
						//IF it is confirmed.
							close_all();
							printf("Thanks For using ISSAC's Chatting Prog\n");
							exit(1);
						}
					}else{
						printf("GOOD BYE!\n");
						exit(1);
					}
				}else if(strcmp(tokens[0], "SEND") == 0 && numTok > 3){
					if(REGISTERED==TRUE){
						int i;
						message[0]='\0';
						//init message
						for(i =2; i<numTok; i++){
							if(strlen(message)+strlen(tokens[i]) < MAX_MSG){
								sprintf(message+strlen(message),"%s ",tokens[i]);
							}else{
								printf("length of message is limited to 100 characters including space!\n");
								continue;
							}
						}
						int index = atoi(tokens[1])-1;
						send(cli_IP[index].sock,message, strlen(message),0);
					}else{
						printf("YOu SHOULD REGISTER and CONNECT first \n");
						continue;
					}		
				}else{
					printf(cli_usage);
				}
			}else{
			//message from other clients or server..
				int t;
				for(t =1; t<MAX_CLI_CONNECT; t++){
				//from first client index of cli_IP
					if(FD_ISSET(cli_IP[t].sock,&tempfds)){			
					//if the socket is triggered..
						int terminator;
						if(t == 1){
						//message from the server
							if((terminator = recv(cli_IP[t].sock, list, MAX_LIST, 0)) != 0){
							//if client can read from server,
								list[terminator]='\0';
								printf(list);
							}else{
							//server is gone.. so as other client..
								printf("\nServer is gone..!!\nLOST ALL CONNECTION..\n");
								// begin initializing..
								maxfd = cli_IP[0].sock;
								//return to initial maxfd
								if(close_all()) printf("CLOSE all Connection\n");
								//destroy all socket except listening one..
								if((cli_IP[1].sock = socket(AF_INET, SOCK_STREAM, 0))==-1){
									perror("SERVER DISASTER\n");
									exit(1);
								}
								cli_IP[1].sock_info.sin_family = AF_INET;
								cli_IP[1].sock_info.sin_port = htons(income_port+1);
								// cli_IP[0] will be uesd to listening port, 
								// cli_IP[1] will be used to communicate to the server
								cli_IP[1].sock_info.sin_addr.s_addr = inet_addr(getLOIP());
						           	if((bind(cli_IP[1].sock, (struct sockaddr*)&cli_IP[1].sock_info, sock_len)) == -1){
									perror("Cli- RE-BIND ERR:");
									exit(1);
								}		
								//Restore the socket for the server..
								int i;
								for(i =2; i<MAX_CLI_CONNECT; i++){
									if(cli_IP[i].sock != 0){
										 FD_CLR(cli_IP[i].sock,&readfds);
										 cli_IP[i].sock = 0; 
									}
								//no need to FD_CLR on cli_IP[1].sock.
								//Cleaning up all client setup
								}REGISTERED = FALSE;
								fd_count =0;
								cli_con_max = 1;		
							}	
						}else{
						//message from other clients
							int terminator = recv(cli_IP[t].sock, message, MAX_MSG, 0);
							if(terminator != 0){
								message[terminator]='\0';
								printf(message);
							}else{
								printf("CLIENT LEFT!!\n");
								maxfd =	findMaxFd(cli_IP[t].sock,maxfd);
								//find next highest socket and assign to maxfd							
								close(cli_IP[t].sock);
								//finish the socket
								FD_CLR(cli_IP[t].sock,&readfds);
								//unmark from readfds
								cli_IP[t].sock = 0;
								//init socket number.
								LIST();
								//update list and
								cli_con_max--;
							}
						}
					}
					if(fd_count-- == 0) break;
					//need to check so that another epoch of select could excute
				}
			}
		}			
	}
}
void init(){	
	FD_ZERO(&readfds);
	//init fd_set
	
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
		int i;
		for(i = 0; i <2; i++){
		//I am making two sockets here and binding both of them.
			if((cli_IP[i].sock = socket(AF_INET, SOCK_STREAM, 0))==-1){
				perror("ERROR! SOCKCREATE-cli :");
				exit(1);
			}
			cli_IP[i].sock_info.sin_family = AF_INET;
			if(i ==0)cli_IP[i].sock_info.sin_port = htons(income_port);
			else cli_IP[i].sock_info.sin_port = htons(income_port+1);
			// cli_IP[0] will be uesd to listening port, 
			// cli_IP[1] will be used to communicate to the server
			cli_IP[i].sock_info.sin_addr.s_addr = inet_addr(getLOIP());
	           	if((bind(cli_IP[i].sock, (struct sockaddr*)&cli_IP[i].sock_info, sock_len)) == -1){
				perror("Cli- BIND ERR:");
				exit(1);
			}
		}
		if(listen(cli_IP[0].sock, BACKLOG) == -1){
			perror("Client listen");
			exit(1);
		}
		cli_con_max = 1;
		break;
	}
}
//initializing socket for server or client 
