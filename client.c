#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
	
#define BUFSIZE 1024
#define LENGTH 512
int flag = 1;
char user[15];

void send_recv(int i, int sockfd){
				//indice nell'array che indica chi è
						//socket del server
	char send_buf[BUFSIZE];
	char recv_buf[BUFSIZE];
	int nbyte_recvd;

	if (i == 0){//è il server
		fgets(send_buf, BUFSIZE, stdin);
		//char *fgets(char *s, int size, FILE *stream);
		//La funzione fgets() legge una linea dallo stream immagazzinandola nel buffer puntato da s, della grandezza di size
		
		if (strcmp(send_buf , "quit\n") == 0) {//comando per uscire
			exit(0);
		}else if(strcmp(send_buf,"musica\n")==0){
				//prima devo avvisare il server che ho scelto la musica
				send(sockfd, send_buf, strlen(send_buf), 0);
				
				char revbuf[LENGTH];//recive buffer
				printf("[Client] Ricevimento del file dal server...");
				char* fr_name = "/home/matteo/Desktop/nuvolebianche.mp3";
				FILE *fr = fopen(fr_name, "a");
				if(fr == NULL)
					printf("File %s non può essere aperto.\n", fr_name);
				else{
					bzero(revbuf, LENGTH); 
					int fr_block_sz = 0;
	    			while((fr_block_sz = recv(sockfd, revbuf, LENGTH, 0)) > 0){
						int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
	        			if(write_sz < fr_block_sz){
	            			printf("fallimento nella scrittura del file\n");
	        			}
						bzero(revbuf, LENGTH);
						if (fr_block_sz == 0 || fr_block_sz != 512) {
							break;
						}
					}
					if(fr_block_sz < 0){//c'è un errore
						if (errno == EAGAIN){//errore di tempo scaduto
							printf("recv() tempo scaduto.\n");
						}
						else{//fallito con un errore
							fprintf(stderr, "recv() fallito = %d\n", errno);
						}
					}
	    			printf("Ricevuto dal server!\n");
	    			fclose(fr);
				}

			system("cvlc /home/matteo/Desktop/nuvolebianche.mp3");
			system("clear");
			/*
			qua avrei dovuto dire al server che sono tornato disponibile
			*/
			//send(sockfd, "disponibile", strlen(send_buf), 0);
				
		}else{
		
			int j=0;
			char sponda[1024];
				
			for(j=0; send_buf[j]!='\0';j++)
				sponda[j] = send_buf[j];
				sponda[j]='\0';
			for(j=0; user[j]!='\0';j++)
				send_buf[j] = user[j];
			send_buf[j++] = ':';
			send_buf[j] = ' ';
			
			j=0;
			while(sponda[j]!='\0')
				send_buf[j+strlen(user)-1+2]=sponda[j++];
			send_buf[j+strlen(user)+2]='\0';
			send(sockfd, send_buf, strlen(send_buf), 0);//invio i dati al server
		}
	}else {//questa parte la esegue il client quando deve ricevere i messaggi dal server degli altri utenti e poi li stampa
		nbyte_recvd = recv(sockfd, recv_buf, BUFSIZE, 0);//altrimenti mettiti in ascolto e ricevi i dati dal server e mettili nel buffer recv_buf
		recv_buf[nbyte_recvd] = '\0';//aggiungi la fine 
		printf("%s" , recv_buf);//stampa
		fflush(stdout);
	}
}
		
		
void connect_request(int *sockfd, struct sockaddr_in *server_addr){
						//puntatore al socket	
									//struttura per l'indirizzo server
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {//creo socket di tipo TCP
		perror("Socket");
		exit(1);
	}
	server_addr->sin_family = AF_INET;//uso l'operatore -> per settare i parametri nella struttura dell'indirizzo del server
	server_addr->sin_port = htons(4950);
	server_addr->sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server_addr->sin_zero, '\0', sizeof(server_addr->sin_zero));//imposto la struttura come vuota.
	
	if(connect(*sockfd, (struct sockaddr *)server_addr, sizeof(struct sockaddr)) == -1) {//tenta di conenttersi al server
		perror("connect");
		exit(1);
	}
}
	
int main(){
	int sockfd, fdmax, i;
	struct sockaddr_in server_addr;
	fd_set master;
	fd_set read_fds;
	
	system("clear");
	if (flag){//per inserire l'username
		printf("Scegliere uno username di massimo 15 caratteri: ");
		fgets(user,15,stdin);//fgets inserisce nella stringa il valore 	\n finale
		user[strlen(user)-1]='\0';//allora qua tolgo quel valore mettendo il terminatore prima
		printf("Ciao %s!\n",user);
		flag=0;
	}
	
	connect_request(&sockfd, &server_addr);
	FD_ZERO(&master);//setto il file descriptor master con tutti zeri
        FD_ZERO(&read_fds); //setto il file descriptor della lettura read_fds con tutti zeri 
        FD_SET(0, &master);	//setto il valore in posizine zero per il file descriptor del master
        FD_SET(sockfd, &master); //setto il valore del socket nel file descriptor del master.
	
	fdmax = sockfd;//assegno come socket massimo quello del server
	
	while(1){
		read_fds = master;//setto il file descriptor master come file descriptor per la lettura.
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){//avvio la select, la select si blocca fino a quado uno tra i descrittori dei socket non è pronto per la lettura.
			perror("select");
			exit(4);
		}
		
		for(i=0; i <= fdmax; i++)
			if(FD_ISSET(i, &read_fds))
				send_recv(i, sockfd);
							//passo chi è(indice nell'array)
								//passo il socket del server
	}
	printf("client-quited\n");
	close(sockfd);
	return 0;
}
