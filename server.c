#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
	
#define PORT 4950
#define BUFSIZE 1024 //lunghezza dei buffer di chat
#define LENGTH 512	//lunghezza dei buffer di invio canzoni

/*avrei dovuto fare una struttura che gestisce gli utenti*/


/*avrei dovuto fare una funzione per la gestione della musica*/


//funzione per vedere se c'è una stringa dentro un altra stringa
int strpos(char *str, char *search){
    //dichiarazioni
    int i,j,n,l;
     
    //inizializzazioni
    l = strlen(search);
    n = strlen(str)-l+1;
     
    //calcoli
    for(i=0;i<n;i++){
        for(j=0;j<l&&search[j]==str[i+j];j++);
        if(j==l)
                return 1;
    }
    return 0;               
}

//funzione che serve per innoltrare a tutti i messaggi spediti dagli altri utenti
void send_to_all(int j, int i, int sockfd, int nbytes_recvd, char *recv_buf, fd_set *master){
					//j dice a chi inviarlo, 
						//i chi l'ha inviato, 
								//sockfd è il socket master, 
												//nbytes_recvd dice quanti byte sono inviati
																//rec_buf è il buffer che contiene il messaggio
																			//master è il file descriptor del master
	if (FD_ISSET(j, master)){
		if (j != sockfd && j != i) {//se j non è il master o chi ha inviato il messaggio invio il messaggio.
			if (send(j, recv_buf, nbytes_recvd, 0) == -1) {
				perror("send");
			}
		}
	}
}

//funzione che quello che riceve 	
void send_recv(int i, fd_set *master, int sockfd, int fdmax){
				//numero in array che dice quale fd è pronto a leggere
						//puntatore del file descriptor master
										//socket del master
														//numero di socket massimo
	int nbytes_recvd, j;
	char recv_buf[BUFSIZE];
	
	if ((nbytes_recvd = recv(i, recv_buf, BUFSIZE, 0)) <= 0) {
		if (nbytes_recvd == 0) {
			printf("socket %d staccato\n", i);
		}else {
			perror("recv-errore");
		}
		close(i);
		FD_CLR(i, master);
	}else { 
		//printf("%s", recv_buf);

		//if(strpos(recv_buf, "disponibile")){//controllo se il client è tornato disponibile dopo aver ascoltato la musica
		//	/* qua avrei dovuto gestire il codice di quando un client tornava disponibile*/	
		//}
		if(strpos(recv_buf, "musica\n")){//conrollo se è stata richiesta l'opzione della musica
			char* fs_name = "/home/matteo/Desktop/multichat_socket/nuvolebianche.mp3";
		    char sdbuf[LENGTH]; // send buffer
		    printf("[Server] Invio %s al Client...", fs_name);
		    FILE *fs = fopen(fs_name, "r");
		    if(fs == NULL){
		        printf("[Errore]: File %s non trovato nel server.\n", fs_name);
				exit(1);
		    }

		    bzero(sdbuf, LENGTH); 
		    int fs_block_sz; 
		    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0){
	/*qua ho messo i al posto che nsockfd*/	        if(send(i, sdbuf, fs_block_sz, 0) < 0)
		        {
		            printf("[Errore]: Fallimento di invio del file %s. \n", fs_name);
		            exit(1);
		        }
		        bzero(sdbuf, LENGTH);
		    }
		    printf("[Ok] inviato il file al client!\n");
		}else{
				for(j = 0; j <= fdmax; j++){
					send_to_all(j, i, sockfd, nbytes_recvd, recv_buf, master );//invoco la fuzione per inviare a tutti i messaggi
								//j dice a chi inviarlo, 
									//i chi l'ha inviato, 
										//sockfd è il socket master, 
												//nbytes_recvd dice quanti byte sono inviati
															//rec_buf è il buffer che contiene il messaggio
																	//master è il file descriptor del master
				}//ciclo for
			}//chiusura else
	}//chiusura elese	
}



//funzione che serve per accettare le connessioni dei nuovi utenti che si vogliono connettere
void connection_accept(fd_set *master, int *fdmax, int sockfd, struct sockaddr_in *client_addr){
						//puntatore al file descriptor master
										//puntatore al socket del fd maggiore
													//socket del master
																	//puntatore a struttura per l'indirizzo client
	socklen_t addrlen;
	int newsockfd;
	
	addrlen = sizeof(struct sockaddr_in);
	if((newsockfd = accept(sockfd, (struct sockaddr *)client_addr, &addrlen)) == -1) {//accetto la richiesta della nuova connessione
		perror("accept-error");
		exit(1);
	}else {		//ora con FD_SET setto nell'array del master il bit del nuovo socket del client
		FD_SET(newsockfd, master);//FD_SET(int fd, fd_set* fdset) il primo indica la posizione nell'array e il secondo passa l'array
		if(newsockfd > *fdmax){
			*fdmax = newsockfd;
		}
													//stampo l'indirizzo del client    //stampo la porta del client
		printf("nuova connessione da: %s sulla porta %d \n",inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
	}
}




//crea il socket
//setta la porta e l'indirizzo di se stesso
//fa la bind ovvero lega al socket la porta
//fa la listen() che definisce la coda di richieste massima di tcp
void connect_request(int *sockfd, struct sockaddr_in *my_addr){
						//dove mettere il socket
										//indirizzo del server
	int yes = 1;
		
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket-error");//La funzione di libreria C void perror (const char * str) stampa un messaggio di errore descrittivo su stderr. Prima viene stampata la stringa str , seguita da due punti, quindi da uno spazio.
		exit(1);
	}
		/*
		
		Esempio:
			typedef struct{ int Campo_1,Campo_2; } TipoDato;
			TipoDato S, *P;
			P = &S;
			(*P).Campo1=75; /* assegnamento della costante 75 al Campo1 della
			struct puntata da P* (e` necessario usare le
			parentesi) /
		
		Operatore ->:
				L'operatore -> consente di accedere ad un campo di una struttura referenziata da un puntatore
								in modo più sintetico: 
				P->Campo1=75;
		*/
	my_addr->sin_family = AF_INET;
	my_addr->sin_port = htons(4950);
	my_addr->sin_addr.s_addr = INADDR_ANY;
	memset(my_addr->sin_zero, '\0', sizeof(my_addr->sin_zero));//setto a zero il valore sin_zero della sockaddr_in dell'ip del server
		
		//getsockopt, setsockopt - ottieni e imposta le opzioni sui socket
		/*
		getsockopt () e setsockopt () manipolano le opzioni per il socket a cui fa riferimento il descrittore di file sockfd . 
		Le opzioni possono esistere a più livelli di protocollo; sono sempre presenti al livello più alto del socket.
		Quando si manipolano le opzioni socket, è necessario specificare il livello in cui risiede l'opzione e il nome dell'opzione. 
		Per manipolare le opzioni a livello di API socket, il livello è specificato come SOL_SOCKET . 
		Per manipolare le opzioni a qualsiasi altro livello viene fornito il numero di protocollo del protocollo appropriato che controlla l'opzione. 
		Ad esempio, per indicare che un'opzione deve essere interpretata dal protocollo TCP , il livello deve essere impostato sul numero di protocollo TCP ; 
		vedi getprotoent (3).
		
		int getsockopt (int sockfd, int level, int optname, void * optval, socklen_t * optlen)
		int setsockopt (int sockfd, int level, int optname, const void * optval, socklen_t optlen);
				
				sockfd->socket
				level-> livello
				optname->opzione name 
				optval->usati per accedere ai valori delle opzioni per setsockopt (). 
						Per getsockopt () identificano un buffer in cui deve essere restituito il valore per le opzioni richieste 
				socklen_t->grandezza del socket
				
				
				usare SO_REUSEADDR in modo da poter continuare a utilizzare la porta anche quando si verifica l'errore 
				SO_REUSEADDRè più comunemente impostato nei programmi server di rete, poiché un modello di utilizzo comune è quello di apportare una modifica 
				alla configurazione, quindi è necessario riavviare quel programma per rendere effettiva la modifica. 
				Senza SO_REUSEADDR, la bind()chiamata nella nuova istanza del programma riavviato fallirà se ci fossero connessioni aperte all'istanza precedente quando l'hai uccisa. 
		*/
	if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt-errore");
		exit(1);
	}
	
	if (bind(*sockfd, (struct sockaddr *)my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind-errore");
		exit(1);
	}
	if (listen(*sockfd, 10) == -1) {//definisce la lunghezza massima della coda di un socket dove si inseriscono le richieste in coda che il server non ha avuto ancora modo di processare.
		perror("listen-errore");
		exit(1);
	}
	printf("\n[Server-TCP] attende per un client alla porta 4950\n");
	fflush(stdout);//pulisce lo stdout scaricando i dati bufferizzati sullo stream stdout
}
int main(){
	fd_set master;
	fd_set read_fds;
	int fdmax, i;
	int sockfd = 0;
	struct sockaddr_in my_addr, client_addr;
	
	system("clear");
	
	FD_ZERO(&master);//setto a zero il file descriptor del master
	FD_ZERO(&read_fds);//setto a zero il file descriptor read_fds che serve per la lettura
	connect_request(&sockfd, &my_addr);//funzione che avvia il socket server
	FD_SET(sockfd, &master);//FD_SET(int fd, fd_set* fdset) il primo indica la posizione nell'array e il secondo passa l'array
	
	fdmax = sockfd;//qua sockfd diventa il sockfd del master perchè c'è il puntatore dentro connect_request che lo imposta
	while(1){
		read_fds = master;//passo il buffer fd_set del buffer a read_fds che è il buffer di lettura.
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
			perror("select-error");
			exit(4);
		}
		
		for (i = 0; i <= fdmax; i++){//ciclo in continuazione per vedere se c'è qualche client libero.
			if (FD_ISSET(i, &read_fds)){//controllo se è settato il bit per questo client
				if (i == sockfd)//sockfd è il socket del master, vuol dire che un nuovo utente l'ha contattato se vuole scrivere
					connection_accept(&master, &fdmax, sockfd, &client_addr);
				else
					send_recv(i, &master, sockfd, fdmax);//questa funzione serve per leggere cosa gli utenti hanno scritto e poi invoca un altra funzione per condividere il messaggio a tutti.
			}
		}
	}
	return 0;
}
