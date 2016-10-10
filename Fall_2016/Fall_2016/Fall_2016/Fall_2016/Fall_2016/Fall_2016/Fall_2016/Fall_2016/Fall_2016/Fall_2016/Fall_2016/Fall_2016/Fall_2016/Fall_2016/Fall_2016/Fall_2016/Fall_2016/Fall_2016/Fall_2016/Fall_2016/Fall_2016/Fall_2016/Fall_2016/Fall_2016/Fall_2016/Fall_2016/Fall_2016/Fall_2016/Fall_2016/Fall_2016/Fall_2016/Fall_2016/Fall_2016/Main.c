#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Main.h"

unsigned short int income_port;
char mode;
//defined in Main.h, for saving argument

int main(int argc, char *argv[]){

	//argument count checking
	if(argc!=3){
		printf("You need two argument nomore, noless\n");

	//if there are two arguments
	}else{
		//check if first argumenmt shows valid input
		if(strcmp("s",argv[1])==0 || strcmp("c",argv[1])==0){
			mode = *argv[1];

			//check if second argument is valid input
			int temp = atoi(argv[2]);
			if(temp > 65535 || temp < 1025){
				printf("you need to type valid port number!!: 1024 < x < 65535 [port number over 1024 is recommended]\n ");
			}else{
				income_port = temp;
				
				//if all valid, run shell!
				shell();
			}
		}else{
			printf("You need to specify mode of the program : either 's' - server or 'c' - client\n");
		}
	}
	return 0;

}
