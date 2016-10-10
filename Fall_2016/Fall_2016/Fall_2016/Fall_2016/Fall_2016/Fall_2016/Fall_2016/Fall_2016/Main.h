#ifndef _Main_h_
#define _Main_h_

#define TRUE 1
#define FALSE 0
/*make boolean values*/

#define BUF_SIZE 512
#define NUM_TOK 16
/*Set proper size of buffer and number of tokens*/


extern unsigned int numTok;
/*numTok represents number of Token read from users*/

extern unsigned short int income_port;
/*port represents port number as an argument of program.*/

extern char mode;
/*mode represents if an user is whether client or server*/

void shell(void);

#endif
