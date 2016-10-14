#include "network.h"
#include <stdio.h>
#include <string.h>

unsigned int numTok;
char buffer[MAX_COMMAND];
/*Buffer to save user input.*/

char *tok[40];
/*array of saving string and number of tokens*/

char* readLine(void){

	fgets(buffer, MAX_COMMAND, stdin);
	/*fgets method will store input from stdin to buffer thats size of BUF_SIZE*/
	if(strlen(buffer) != 1){
		buffer[strlen(buffer)-1] = '\0';
	}else{
		return '\0';
	}
	/*Since the last character in buffer is '\n' 
	  it should be changed to nullcharacter which is '\0'*/
	return buffer;
}
/*readLine method will read input until user hit '\n' character*/

char **parseTok(char* input){
	/*int i is for indexing of array and saving number of token*/
	numTok=0;
	
	tok[numTok++] = strtok(input," ");
	/*first token is saved to tok[0] then increment i by 1*/
	
	while((tok[numTok]=strtok(NULL," "))!=NULL){
	/*This while loop will iterate tokens in input until there is no token left
	  each token will be saved to temp*/
		numTok++;
		/*increse numTok by 1*/	
		if(numTok>=40){
			printf("Please use less than 40 space total when typing" );
			return NULL;
		}
	}
	return tok;
}
/*buffer from readLine is parameter of parseTok.
  this method will generate an array of tokens delimed by '\s' 
void shell(void){
	
	int exit = FALSE;
	char *line;
	char **tokens;
	init();	
	
	for(;!exit;){
		if(mode=='s'){
			printf("Welcome Server mode!\n");
			exit = runServ();	
			printf("My Job is done\n");
		}else{
			printf("PA1> ");
			line = readLine();
			
			tokens = parseTok(line);
			exit = cli_parse(tokens);
			
			numTok = 0;
		}	}
	}
}
*/
