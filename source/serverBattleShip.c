#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


#define PERMISOS 0600

key_t llave;
int idCola;

typedef struct{
	long tipo;
	int fila;
	int col;
	int atino;
	int ganaste;
}msg;

int longitud = sizeof(msg) - sizeof(long);

void crearCola(void);
void infCola(struct msqid_ds *);
void leerMensaje(int,msg *);

int main(int argc, char **argv){
	char *arg[] = {"/usr/bin/gnome-terminal" , "-e","./cliente" , NULL};
	int i;
	msg mensaje;
	struct msqid_ds buf; //informacion sobre la cola

	//Crear un llave en el la ruta actual 
	llave = ftok("/tmp",120); 
	crearCola();
	msgctl( idCola, IPC_RMID, 0);
	crearCola();
	
	//Levantando Clientes para iniciar el juego	
	for(i = 1 ; i<= 2; i++){
		if( fork() == 0){
			execvp(arg[0], arg);
			exit(0);
		}
	}
	return 0;
}


void infCola(struct msqid_ds *buf){
	msgctl(idCola, IPC_STAT,buf);
}

void enviarMensaje(msg m){
	if (msgsnd(idCola, &m, longitud, 0) == -1) {
		perror("Error al enviarmensaje:" );
	}
}


void crearCola(void){
	idCola = msgget(llave, IPC_CREAT | PERMISOS);
	if(idCola == -1){
		perror("Opss!: ");
	}
}
void leerMensaje(int tipo, msg *mensaje){
	if (msgrcv (idCola, mensaje, longitud, tipo, IPC_NOWAIT ) == -1){
		perror("Error al recbir:" );
	}
}

