#include "Main.h"
#include "network.h"
#include <stdio.h>
#include <string.h>

unsigned int numTok=0;
char buffer[BUF_SIZE];
/*Buffer to save user input.*/

char *tok[NUM_TOK];
/*array of saving string and number of tokens*/

char* readLine(void){
	
	fgets(buffer, sizeof(buffer), stdin);
	/*fgets method will store input from stdin to buffer thats size of BUF_SIZE*/
	
	buffer[strlen(buffer)-1] = '\0';
	/*Since the last character in buffer is '\n' 
	  it should be changed to nullcharacter which is '\0'*/
	  
	return buffer;
}
/*readLine method will read input until user hit '\n' character*/

char **parseTok(char* input){
	/*int i is for indexing of array and saving number of token*/
	  
	tok[numTok++] = strtok(input," ");
	/*first token is saved to tok[0] then increment i by 1*/
	
	while((tok[numTok]=strtok(NULL," "))!=NULL){
	/*This while loop will iterate tokens in input until there is no token left
	  each token will be saved to temp*/
	  
		numTok++;
		/*increse numTok by 1*/	
			
		if(numTok>=NUM_TOK){
			printf("Please use less than %d space when typing", NUM_TOK);
			return NULL;
		}
	}
	return tok;
}
/*buffer from readLine is parameter of parseTok.
  this method will generate an array of tokens delimed by '\s' */
void shell(void){
	
	int exit = FALSE;
	char *line;
	char **tokens;
	init();	
	
	for(;!exit;){
	/*while not exited*/
		if(mode=='s'){
			printf("Welcome Server mode!\n");
			exit = runServ();	
			printf("My Job is done\n");
		}else{
			printf("PA1> ");
			line = readLine();
			/*save user input to line*/
			
			tokens = parseTok(line);
			/*seperate each tokens and save them to tokens*/
			exit = cli_parse(tokens);
			/*pass it to parse to execute and set exit if exited*/
			
			numTok = 0;
			/*init numTok for next command*/
		}
	}
}
