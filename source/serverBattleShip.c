#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv){
	char *arg[] = {"/usr/bin/gnome-terminal" , "-e","./cliente" , NULL};
	int i;

	//Levantando Clientes para iniciar el juego	
	for(i = 1 ; i<= 2; i++){
		if( fork() == 0){
			execvp(arg[0], arg);
			exit(0);
		}
	}
	return 0;
}


