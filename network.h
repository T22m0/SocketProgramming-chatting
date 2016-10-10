/*One thing I noticed was that Sockaddr_in should not be declared by pointer..
 * or it will mess up all things.*/
#ifndef _NETWORK_H_
#define _NETWORK_H_
#include <netinet/in.h>
//To use sockaddr_in

#define MAX_CONNECT 21
#define MAX_CLI_CONNECT 4  
#define BACKLOG 10
#define MAX_MSG 100
#define MAX_LIST 512
//server\
_______________________________________//
typedef struct{
	int sock;
	struct sockaddr_in sock_info;
}server;
extern int num_connection;
//counter of connection to server

//Client\
________________________________________________//

extern int cli_con_max;
//counter of connection of clients. limited to 3


char* getIP(void);
void init(void);
int cli_parse(char **token);
int runServ(void);

#endif
