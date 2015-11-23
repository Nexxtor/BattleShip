#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//Escribir y leer
#define PERMISOS 0600
//Tipos de mensaje
#define ESTOY_LISTO 1
#define CONFIRMAR 2

//Constantes utiles
#define TAM_TABLERO 4
#define CANT_BARCOS 3

int idCola;
key_t llave;

typedef struct{
	long tipo;
	int fila;
	int col;
	int atino;
	int ganaste;
}msg;

int longitud = sizeof(msg) - sizeof(long);
int tablero[TAM_TABLERO ][TAM_TABLERO ];
int tableroEn[TAM_TABLERO ][TAM_TABLERO ];

void enviarMensaje(msg);
void crearCola(void);
void infCola(struct msqid_ds *);
void leerMensaje(int,msg *);
void iniciarTablero();
void ponerBarcos();
void atacar();
void mostrarTablero();
int verificarGane();
 
int main(int argc, char **argv){
	msg mensaje;
	struct msqid_ds buf; //informacion sobre la cola
	int oponente, turno;

	
	//Crear un llave en el la ruta actual 
	llave = ftok("/tmp",300000); 
	
	crearCola();
	mensaje.tipo = ESTOY_LISTO + getpid(); //Para saber quien es
	
	enviarMensaje(mensaje);
	system("clear");
	printf("Esperando jugador\n");
	//Esperando que los dos clientes esten levantados
	do{
		infCola(&buf);
	}while(buf.msg_qnum != 2 && buf.msg_lrpid == 0 );
	
	//Si fue el ultimo cliente en llegar
	if(buf.msg_lspid == getpid()){
		leerMensaje(0,&mensaje); // leo el primer mensaje
		oponente = mensaje.tipo - ESTOY_LISTO;		
		turno = 0;
	}else{
		do{
			infCola(&buf);
		}while(buf.msg_qnum != 1);
		leerMensaje(0,&mensaje); // leo el primer mensaje
		oponente = mensaje.tipo - ESTOY_LISTO;		
		turno = 1;
	}
	//A jugar;
	
	iniciarTablero();
	ponerBarcos();

	while(1){
		verificarGane();
		mostrarTablero();		
		if(turno == 0){
			printf("Esperando Ataque..\n");
			leerMensaje(oponente,&mensaje);
			mensaje.tipo = getpid() + CONFIRMAR;
			if(haceEfectivoAtaque(mensaje.fila,mensaje.col)){
				printf("Nos Dieron\n");
				mensaje.atino = 1;
			}else{
				printf("Fallaron\n");
				mensaje.atino = 0;
			}		
			enviarMensaje(mensaje);

			turno = 1;
		}else{
			atacar(oponente);
			leerMensaje(oponente + CONFIRMAR,&mensaje);
			if(mensaje.atino == 1){
				printf("Le hemos dado\n");
				tableroEn[mensaje.fila][mensaje.col] = 'Y';
			}else{
				printf("No le dimos\n");
				tableroEn[mensaje.fila][mensaje.col] = 'X';
			}

			turno = 0;
		}
		

		
	}
	
	
	
	return 0;

}

int verificarGane(){
	int gano = 0, gane = 0 , i ,j;
	for(i=0; i<TAM_TABLERO ; i++){
		for(j=0;j<TAM_TABLERO ; j++){	
			if(tablero[i][j] == 'Y'){
				gano ++;
			}
			if(tableroEn[i][j] == 'Y'){
				gane++;
			}
		}
	}
	
	if(gane == CANT_BARCOS || gano == CANT_BARCOS ){
		if(gane == CANT_BARCOS) printf("ganamos\n"); else printf("pedimos");
		getchar();
		getchar();
		msgctl( idCola, IPC_RMID, 0);
		exit(0);
	}
	
	return -1;
}
int haceEfectivoAtaque(int i, int j){
	
	if(tablero[i][j] == 'v' ){
		tablero[i][j] = 'Y';
		return 1;
	}
	tablero[i][j] = 'F';
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
	if (msgrcv (idCola, mensaje, longitud, tipo, 0 ) == -1){
		perror("Error al recbir:" );
	}
}


void iniciarTablero(){
	int i,j;
	for(i=0; i<TAM_TABLERO ; i++){
		for(j=0;j<TAM_TABLERO ; j++){
			tableroEn[i][j] = tablero[i][j] = ' ';
		}
	}

}


void ponerBarcos(){
	int cantPuestos = 0 ,i,j;
	char c;

	printf("Indicaciones:\nEscriba numero del 1 al %d para seleccionar fila\n" , TAM_TABLERO );
	printf("Escriba letras desde A hasta %c para seleccionar columna\n" , 'A' + TAM_TABLERO );
	while(cantPuestos != CANT_BARCOS){
		printf("\nSeleccione donde ubicar su barco numero %d.\n Fila: ", cantPuestos  + 1);
		scanf("%d", &i); // fila
		printf(" Columna: ");
		scanf("\n%c",&c); //Limpiar buffer
		j = -('A'-c);
		i = i -1;
		if(i <0 || i >TAM_TABLERO || j <0 || j > TAM_TABLERO){
			printf("Cordenadas invalidas ..\n");
		}else{
			if(tablero[i][j]  != 'v'){
				tablero[i][j]= 'v';
				cantPuestos++;
			}else{
				printf("Ya hay barco en esas cordenas\n");
			}
			
		}
	}
	
	
	
}

void atacar(){
	int i,j,ok = 0;
	char c;
	msg m;

	while(ok == 0){
		printf("A donde deseas atacar\n Fila: ");
		scanf("%d", &i); // fila
		printf(" Columna: ");
		scanf("\n%c",&c); //Limpiar buffer	
		j = -('A'-c);
		i = i -1;
		printf("%d, %d\n",i,j);
		if(i <0 || i >TAM_TABLERO || j <0 || j > TAM_TABLERO){
			printf("Cordenadas invalidas ..\n");
		}else{
			break;
			printf("Listo %d\n" ,ok);
		}
	}
	tableroEn[i][j] = 'X';
	m.tipo =  getpid();
	m.fila = i;
	m.col = j;
	m.ganaste = 0;
	
	enviarMensaje(m);

}
void mostrarTablero(){
	int i,j;
	printf("\n tablero de ataque a enemigo \n");
	for(i=0; i<TAM_TABLERO ; i++){
		printf(" %c |", 'A' +i);
	}
	printf("\n");
	for(i=0; i<TAM_TABLERO ; i++){
		printf("%d ", i);
		for(j=0;j<TAM_TABLERO ; j++){
			printf(" %c |", tableroEn[i][j] );
		}
		printf("\n");
	}
	printf("\n Tu tablero\n");
	for(i=0; i<TAM_TABLERO ; i++){
		printf(" %c |", 'A' +i);
	}
	printf("\n");
	for(i=0; i<TAM_TABLERO ; i++){
		printf("%d ", i);
		for(j=0;j<TAM_TABLERO ; j++){
			printf(" %c |", tablero[i][j] );
		}
		printf("\n");
	}
	
}
