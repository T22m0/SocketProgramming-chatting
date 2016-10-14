/*One thing I noticed was that Sockaddr_in should not be declared by pointer..
 * or it will mess up all things.*/
#ifndef _NETWORK_H_
#define _NETWORK_H_
#include <netinet/in.h>
//To use sockaddr_in

#define MAX_CONNECT 21
#define MAX_CLI_CONNECT 5 
#define BACKLOG 10
#define MAX_MSG 100
#define MAX_LIST 512
#define MAX_COMMAND 128

#define TRUE 1
#define FALSE 0
/*make boolean values*/

extern unsigned int numTok;
/*numTok represents number of Token read from users*/

extern unsigned short int income_port;
/*port represents port number as an argument of program.*/

extern char mode;
/*mode represents if an user is whether client or server*/

typedef struct{
	int sock;
	struct sockaddr_in sock_info;
}sock;

//server\
_______________________________________//
extern int num_connection;
//counter of connection to server

//Client\
________________________________________________//

extern int cli_con_max;
//counter of connection of clients. limited to 3

int runCli(void);
int runServ(void);
void init(void);

char* readLine(void);
char** parseTok(char* input);
#endif
