#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0



// Lista subscriptores
typedef struct suscriptor_Struct{
  int socket;
  char* direccion;
  struct suscriptor_Struct* next;
}Subscriptor;
typedef struct ListaIdentificar2 {
		Subscriptor *inicio;
		int tam;
}Lista_subs;

typedef struct tema_Struct{
  char* nombre; 
  Lista_subs *subs;  
  struct tema_Struct *next;
}Tema;
typedef struct ListaIdentificar {
		Tema *inicio;		
		int tam;		
}Lista_temas;

int buscar_tema(Lista_temas *temas,const char *tema,Tema **vocero);
int alta_subscriptor( Lista_subs **subs, const char* tema,int puerto,char* direccion);
void insertar(Lista_temas* lista,char* tema);
void insertar_sub(Subscriptor *s);
int eliminar_sub(Lista_subs *ls,int valor);
void inicializacion_temas (Lista_temas *lista);
void inicializacion_subs (Lista_subs *lista);
int crear_socket_TCP(int puerto);
void escribir_tema(FILE* fd, char* tema);
int baja_subscriptor(Lista_temas *temas,const char* tema,int valor);
int abrir_conexion_tcp_con_servidor(int puerto_servidor, char* direccion_subscriptor);

#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
Lista_temas temas;
Lista_subs subs;

int main(int argc, char *argv[]) {
  int sock_tcp, new_fd;/* listen en sock_tcp, nuevas conexiones en new_tcp*/
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  char s[INET6_ADDRSTRLEN];

  FILE *fd;
  char line[100];
  //size_t len = 0;
  //ssize_t read;
  
  inicializacion_temas(&temas);
  inicializacion_subs(&subs);
  
	if (argc!=3) {
		fprintf(stderr, "Uso: %s puerto fichero_temas\n", argv[0]);
		return 1;
	}
	//Crear socket
	sock_tcp = crear_socket_TCP(atoi(argv[1]));
	//Crear nuevo fichero para guardar los temas
	fd = fopen(argv[2],"r+");
	
		
	Tema* actual;
	Tema* aux = malloc(sizeof(Tema));
	temas.tam = 0;
	int n;
	// Incluír en la estructura de datos los temas
	while (fgets(line, 100, fd) != NULL){
	//Crear nuevo tema
	n = (int) strlen (line);
	line[--n] = '\0';
	insertar(&temas,line);
  }
  
  if (listen(sock_tcp, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    
    while(1) {  // main accept() loop
    printf("\nSERVER: waiting for connections...\n");
        sin_size = sizeof their_addr;
        new_fd = accept(sock_tcp, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);
		char buf[128] = "";
		char tema[128];
		char valor[128];
		int op;
		int recived;
		char *resp;
		
		/* COMUNICACIÓN*/
		// PRIMER CAMPO (tema)
		recived=recv(new_fd, buf, 128,0);
		if (recived<0){
			perror("Error recv 1\n");
			close(new_fd);
			exit(0);
		}
		buf[recived] = '\0';
		
		strcpy(tema,buf);
		printf("Recibido: %s\n",tema);
		resp=malloc(128);	
		strcpy(resp,"ok");	
		if (send(new_fd,resp,strlen(resp),0)<0)
		{//incorrecto
			perror("Error send 1\n");
			close(new_fd);
			exit(0);
		}		
		free(resp);	
		// SEGUNDO CAMPO (valor)
		recived=recv(new_fd, buf, 128,0);
		if (recived<0){
			perror("Error recv 2\n");
			close(new_fd);
			exit(0);
		}
		buf[recived] = '\0';		
		strcpy(valor,buf);
		printf("Recibido: %s\n",valor);
		resp=malloc(128);
		strcpy(resp,"ok");	
		if (send(new_fd,resp,strlen(resp),0)<0)
		{//incorrecto
			perror("Error send 2\n");
			close(new_fd);
			exit(0);
		}		
		free(resp);	
		// TERCER CAMPO (operacion)
		
		recived=recv(new_fd, &op, 4,0);
		if (recived<0){
			perror("Error recv 3\n");
			close(new_fd);
			exit(0);
		}		
	
		printf("Recibido: %d\n",op);
		resp=malloc(128);
		strcpy(resp,"ok");	
		if (send(new_fd,resp,strlen(resp),0)<0)
		{//incorrecto
			perror("Error send 3\n");
			close(new_fd);
			exit(0);
		}	
		free(resp);	
		
		int opa = op;
		int t = 0;
		int encontrado = 0;
		int st = 0;
		Subscriptor *sux,*sux2,*nuevo;
		Lista_subs ls;
		inicializacion_subs (&ls);
		Tema *aux2 = malloc(sizeof(Tema*));
		int socket_subs;
		int size;
		char *respuesta;
		int recibidos;
		int subs_tam;
		int eliminado;
		int r2;
		Subscriptor *aus = malloc(sizeof(*aus));
		switch(opa){/* 0:Nuevo_subscriptor, 1:Crear Tema, 2:Eliminar tema, 3:Alta, 4:Baja, 5:Generar evento*/
			case 0: 
			  printf("\nNUEVO SUBSCRIPTOR --------- \n");
			  // Crear subscriptor
			  nuevo = malloc(sizeof(Subscriptor*));
			  nuevo->socket = atoi(valor);
			  nuevo->direccion = malloc(sizeof(s));
			  nuevo->direccion = s;
			  nuevo->next = NULL;
			  //insertar_subs(Subscriptor **s)
			  insertar_sub(nuevo);
			  resp=malloc(128);	
		    strcpy(resp,"ok");	
		    if (send(new_fd,resp,strlen(resp),0)<0)
		    {//incorrecto
			    perror("Error send 1\n");
			    close(new_fd);
			    exit(0);
		    }		
		    free(resp);	
			  printf("Nuevo: Subscriptor insertado\n");
			  //por cada tema{
			  aux2 = temas.inicio;
			  while(aux2!=NULL){			    
			    //  enviar al subscriptor una notificación con su nombre
			    socket_subs = abrir_conexion_tcp_con_servidor(nuevo->socket, nuevo->direccion);
							size = strlen(aux2->nombre);
	            if (send(socket_subs,aux2->nombre,size,0)>=0)
	            {//correcto
		            printf("Nuevo: Envio: %s %d\n",aux2->nombre,size);
	            }else{//mal
		            close(socket_subs);//cerrar conexión
		            perror("Nuevo: Error en el send \n");
		            exit(-1);
		
	            }
	            respuesta = malloc(128);
	            recibidos=recv(socket_subs,respuesta,2,0);
	            if (recibidos<0){
		            close(socket_subs);//cerrar conexión
		            exit(-1);
	            }
	            respuesta[recibidos]='\0';
	            if(strcmp(respuesta,"ok")!=0){
		            //error en la baja del tema
		            fprintf(stderr,"Nuevo: Error al cebar\n");
		            close(socket_subs);
		            return 1;
	            }
	           	
	            strcpy(valor,"cebar");
	            size = strlen(valor);
	            if (send(socket_subs,valor,size,0)>=0)
	            {//correcto
		            printf("Nuevo: Envio: %s %d\n",valor,size);
	            }else{//mal
		            close(socket_subs);//cerrar conexión
		            perror("Nuevo: Error en el send \n");
		            exit(-1);		
	            }
	            printf("Nuevo: Tema enviado al subscriptor\n");
	            recibidos=recv(socket_subs,respuesta,2,0);
	            if (recibidos<0){
		            close(socket_subs);//cerrar conexión
		            exit(-1);
	            }
	            respuesta[recibidos]='\0';
	            if(strcmp(respuesta,"ok")!=0){
		            //error en la baja del tema
		            fprintf(stderr,"Nuevo: Error al cebar\n");
		            close(socket_subs);
		            return 1;
	            }
	            free(respuesta);	
	             close(socket_subs);//cerrar conexión
	            if(aux2->next == NULL){
	                socket_subs = abrir_conexion_tcp_con_servidor(nuevo->socket, nuevo->direccion);
	            	      if (send(socket_subs,"aa",2,0)>=0)
	                    {//correcto
		                   // printf("Nuevo: Envio: %s %d\n",,size);
	                    }else{//mal
		                    close(socket_subs);//cerrar conexión
		                    perror("Nuevo: Error en el send \n");
		                    exit(-1);
		
	                    }
	                    respuesta = malloc(128);
	                    recibidos=recv(socket_subs,respuesta,2,0);
	                    if (recibidos<0){
		                    fprintf(stderr,"Nuevo: Error al cebar\n");
		                    close(socket_subs);//cerrar conexión
		                    exit(-1);
	                    }
	                    respuesta[recibidos]='\0';
	                    if(strcmp(respuesta,"ok")!=0){
		                    //error en la baja del tema
		                    fprintf(stderr,"Nuevo: Error al cebar\n");
		                    close(socket_subs);
		                    return 1;
	                    }
	                   	
	                    //strcpy(valor,"aa");
	                    //size = strlen(valor);
	                    if (send(socket_subs,"aa",2,0)>=0)
	                    {//correcto
		                    //printf("Nuevo: Envio: %s %d\n",valor,size);
	                    }else{//mal
		                    close(socket_subs);//cerrar conexión
		                    perror("Nuevo: Error en el send \n");
		                    exit(-1);		
	                    }
	                    //printf("Nuevo: Tema enviado al subscriptor\n");
	                    recibidos=recv(socket_subs,respuesta,2,0);
	                    if (recibidos<0){
		                    close(socket_subs);//cerrar conexión
		                    exit(-1);
	                    }
	                    respuesta[recibidos]='\0';
	                    if(strcmp(respuesta,"ok")!=0){
		                    //error en la baja del tema
		                    fprintf(stderr,"Nuevo: Error al cebar\n");
		                    close(socket_subs);
		                    return 1;
	                    }
	                    free(respuesta);	
	            
	            }
		         
		          aux2 = aux2->next;
			  }
			  printf("Nuevo: Todos los temas enviados al subscriptor\n");
			  printf("NUEVO: FIN\n");
			break;
			case 1: /* Crear tema */
			  // 0. Comprobar si el tema ya está en la lista de temas
			  printf("\nCREAR: INICIO\n");
			  if(buscar_tema(&temas,tema,NULL)==1){
					printf("Crear: Tema encontrado\n");
					resp=malloc(128);
					strcpy(resp,"encontrado");
					if (send(new_fd,resp,strlen(resp),0)<0)
					{//incorrecto
						perror("Crear: Error send 3\n");
						close(new_fd);
						exit(0);
					}
					printf("Crear: Envio %s %d\n",resp,strlen(resp));
					free(resp);	
					close(new_fd);
					printf("Crear: Fin conexión con editor\n");  
					break;
				}
			  // 1. Actualizar sus estructuras de datos	
			  printf("Crear: Tema no encontrado\n");		  
			  insertar(&temas,tema);
			  resp=malloc(128);
				strcpy(resp,"ok");
				if (send(new_fd,resp,strlen(resp),0)<0)
				{//incorrecto
					perror("Crear: Error send 3\n");
					close(new_fd);
					exit(0);
				}
				printf("Crear: Envio %s %d\n",resp,strlen(resp));
			  free(resp);	
				close(new_fd);
			  printf("Crear: Fin conexión con editor\n");
			  // 2. Avisar a todos los subscriptores 
			  subs_tam = subs.tam;
			  aus = subs.inicio;
			  
			  while(aus!=NULL){
			    //Conectar al subscriptor			
              socket_subs = abrir_conexion_tcp_con_servidor(aus->socket, aus->direccion);
							size = strlen(tema);
	            if (send(socket_subs,tema,size,0)>=0)
	            {//correcto
		            printf("Crear: Envio: %s %d\n",tema,size);
	            }else{//mal
		            close(socket_subs);//cerrar conexión
		            perror("Crear: Error en el send \n");
		            exit(-1);
		
	            }
	            respuesta = malloc(128);
	            recibidos=recv(socket_subs,respuesta,2,0);
	            if (recibidos<0){
		            close(socket_subs);//cerrar conexión
		            exit(-1);
	            }
	            respuesta[recibidos]='\0';
	            if(strcmp(respuesta,"ok")!=0){
		            //error en la baja del tema
		            fprintf(stderr,"Crear: Error al recibir confirmacion\n");
		            close(socket_subs);
		            return 1;
	            }
	            strcpy(valor,"crear");
	            size = strlen(valor);
	            if (send(socket_subs,valor,size,0)>=0)
	            {//correcto
		            printf("Crear: Envio: %s %d\n",valor,size);
	            }else{//mal
		            close(socket_subs);//cerrar conexión
		            perror("Crear: Error en el send \n");
		            exit(-1);		
	            }
	            respuesta = malloc(128);
	            recibidos=recv(socket_subs,respuesta,2,0);
	            if (recibidos<0){
		            close(socket_subs);//cerrar conexión
		            exit(-1);
	            }
	            respuesta[recibidos]='\0';
	            if(strcmp(respuesta,"ok")!=0){
		            //error en la baja del tema
		            fprintf(stderr,"Crear: Error al recibir confirmacion\n");
		            close(socket_subs);
		            exit(-1);
	            }
	            free(respuesta);	
	            close(socket_subs);	
	            aus = aus->next;        		    
			  }
			printf("Crear: Fin conexion subscriptores\n");
			printf("FIN CREAR\n");
			break;
			case 2: /* Eliminar tema */
			  //Cada subscriptor recibe el evento
				aux = malloc(sizeof(Tema*));
				printf("\nELIMINAR: INICIO\n");
				if(buscar_tema(&temas,tema,&aux)==1){
					printf("Eliminar: Tema encontrado en eliminar tema\n");
					resp=malloc(128);
					strcpy(resp,"ok");
					if (send(new_fd,resp,strlen(resp),0)<0)
					{//incorrecto
						perror("Eliminar: Error send 3\n");
						close(new_fd);
						exit(0);
					}
					printf("Eliminar: Envio %s %d\n",resp,strlen(resp));
					free(resp);	
					close(new_fd);
					printf("Eliminar: Fin conexión con editor para eliminar tema\n");
					
					// Enviar a los subscriptores el evento
					
					if(aux->subs!=NULL){
					  printf("DEBUG: AUX->SUBS->TAM %d\n",aux->subs->tam);
						sux = aux->subs->inicio;
						printf("DEBUG---FAIL\n");
						while(sux != NULL){							
							socket_subs = abrir_conexion_tcp_con_servidor(sux->socket, sux->direccion);
							size = strlen(tema);
	            if (send(socket_subs,tema,size,0)>=0)
	            {//correcto
		            printf("Eliminar: Envio tema eliminado a un subscriptor: %s %d\n",tema,size);
	            }else{//mal
		            close(socket_subs);//cerrar conexión
		            perror("Eliminar: Error en el send \n");
		            exit(-1);
		
	            }
	            respuesta = malloc(128);
	            recibidos=recv(socket_subs,respuesta,2,0);
	            if (recibidos<0){
		            close(socket_subs);//cerrar conexión
		            exit(-1);
	            }
	            respuesta[recibidos]='\0';
	            if(strcmp(respuesta,"ok")!=0){
		            //error en la baja del tema
		            fprintf(stderr,"Eliminar: Error al dar de baja\n");
		            close(socket_subs);
		            return 1;
	            }
	            	
	            strcpy(valor,"eliminar");
	            size = strlen(valor);
	            if (send(socket_subs,valor,size,0)>=0)
	            {//correcto
		            printf("Eliminar: Envio valor eliminar al subscriptor: %s %d\n",valor,size);
	            }else{//mal
		            close(socket_subs);//cerrar conexión
		            perror("Eliminar: Error en el send \n");
		            exit(-1);		
	            }
	            respuesta = malloc(128);
	            recibidos=recv(socket_subs,respuesta,2,0);
	            if (recibidos<0){
		            close(socket_subs);//cerrar conexión
		            exit(-1);
	            }
	            respuesta[recibidos]='\0';
	            if(strcmp(respuesta,"ok")!=0){
		            //error en la baja del tema
		            fprintf(stderr,"Eliminar: Error al dar de baja\n");
		            close(socket_subs);
		            return 1;
	            }
	            free(respuesta);	
		          close(socket_subs);//cerrar conexión
		          sux2 = sux;
	            sux = sux->next;
	            //free(sux2);
	            printf("Eliminar: Subscriptor dado de baja en tema %s\n",tema);
						}
						//free(aux->subs);
						printf("Eliminar: Todos los subscriptores han sido eliminados\n");
					}
											// Eliminar el tema de la estructura de datos
						aux = temas.inicio;
						t = 0;
						while(aux!=NULL){
						  if(strcmp(aux->nombre,tema)==0){						   
						    if(t>1){
						      aux2->next = aux->next;						      
						      break;
						    }
						    temas.inicio = aux->next;
						    break;
						  }
						  aux2=aux;
						  aux = aux->next;
						  t++;
						}
						(temas.tam)--;				
						printf("Eliminar: Tema eliminado completamente");
						printf("Eliminar: FIN ELIMINAR\n");
					
					
				}else{
					fprintf(stderr,"Eliminar: Tema no encontrado\n");
					free(resp);
					resp=malloc(128);
					strcpy(resp,"no-encontrado");
					if (send(new_fd,resp,strlen(resp),0)<0)
					{//incorrecto
						perror("Error send 3\n");
						close(new_fd);
						exit(0);
					}
					free(resp);	
					printf("Eliminar: Fin conexión con editor\n");
		  }
      printf("ELIMINAR: FIN\n");
			break;
	case 3: // Alta subs
				t=0;
				encontrado = 0;
			  printf("\nALTA: INICIO ----------- \n");
				printf("Alta: tema %s , subscriptor %d\n",tema,atoi(valor));
				/*while (!encontrado && actual!=NULL){
				  if (strcmp(actual->nombre, tema)==0){
					encontrado = 1;
				  }else{
					  actual = actual->next;
				  }					  
				}*/
				if(buscar_tema(&temas,tema,&actual)==FALSE){
					printf("Alta: Tema No encontrado\n");
					resp=malloc(128);
					strcpy(resp,"error");
					r2 = 0;
					if (send(new_fd,&r2,sizeof(int),0)<0)
					{//incorrecto
						perror("Alta: Error send 3\n");
						close(new_fd);
						exit(0);
					}
					free(resp);	
				}else{
					/*Guardar el puerto del subscriptor*/					
					printf("Alta: Tema Encontrado - actual %s\n",actual->nombre);
					if(alta_subscriptor(&(actual->subs), tema,atoi(valor),s)==TRUE){
					  //El subscriptor no puede subscribirse 2 veces al mismo tema
					  printf("Alta: Subscriptor encontrado ERROR\n");
					  resp=malloc(128);
					  strcpy(resp,"error");
					  r2 = 0;
					  if (send(new_fd,&r2,sizeof(int),0)<0)
					  {//incorrecto
						  perror("Alta: Error send 3\n");
						  close(new_fd);
						  exit(0);
					  }
					  free(resp);	
					  printf("ALTA: FIN ----------- \n");
					  break;
					}
					resp=malloc(128);
					printf("Alta: Subscriptor dado de alta en %s\n",tema);
					strcpy(resp,"ok");
					r2 = 1;
					//if (send(new_fd,resp,strlen(resp),0)<0)
					st= 0;
					while(st == 0){
					  st = send(new_fd,&r2,sizeof(int),0);
					}
					if (st<0)
					{//incorrecto
						perror("Alta: Error send 3\n");
						close(new_fd);
						exit(0);
					}
					free(resp);	
					printf("Alta: Fin conexión\n");
				}
				printf("ALTA: FIN ----------- \n");
			break;
			case 4: // BAJA 
			  printf("\nBAJA: INICIO- --- ----\n");
				//Buscar si existe el tema y si existe eliminar al subsciptor
				if(!baja_subscriptor(&temas,tema,atoi(valor))){
					fprintf(stderr,"Baja subscriptor ERROR.\n");
					resp=malloc(128);
					strcpy(resp,"error");
					r2=0;
					if (send(new_fd,&r2,sizeof(int),0)<0)
					{//incorrecto
						perror("Baja subscriptor Error send 3\n");
						close(new_fd);
						exit(0);
					}
					free(resp);	
					printf("Baja subscriptor Fin conexión\n");
				}else{
					printf("Baja Subscriptor OK.\n");
					resp=malloc(128);
					strcpy(resp,"ok");
					st = 0;
					r2 = 1;
					while(st == 0){
					  st = write(new_fd,resp,strlen(resp));
					}
					if (st<0)
					{//incorrecto
						perror("Baja subscriptor Error send 3\n");
						close(new_fd);
						exit(0);
					}					
					printf("Baja subscriptor Envio %s %d\n",resp,strlen(resp));
					free(resp);	
					printf("Baja subscriptor Fin conexión\n");
				}
				printf("FIN: BAJA ---------------\n");
			break;
			case 5: //Generar Evento
				//Cada subscriptor recibe el evento
				aux = malloc(sizeof(Tema*));
				printf("\nEVENTO: INICIO -----------\n");
				if(buscar_tema(&temas,tema,&aux)==TRUE){
					printf("Evento: Tema encontrado\n");
					resp=malloc(128);
					strcpy(resp,"ok");
					if (send(new_fd,resp,strlen(resp),0)<0)
					{//incorrecto
						perror("Evento: Error send 3\n");
						close(new_fd);
						exit(0);
					}
					printf("Evento: Envio %s %d\n",resp,strlen(resp));
					free(resp);	
					printf("Evento: Fin conexión con editor\n");
					if(aux->subs == NULL){
					  break;
					}
					// Enviar a los subscriptores el evento
					if(aux->subs->tam>0){
						sux = aux->subs->inicio;
						while(sux != NULL){							
							socket_subs = abrir_conexion_tcp_con_servidor(sux->socket, sux->direccion);
							size = strlen(tema);
	            if (send(socket_subs,tema,size,0)>=0)
	            {//correcto
		            printf("Evento: Envio: %s %d\n",tema,size);
	            }else{//mal
		            close(socket_subs);//cerrar conexión
		            perror("Evento: Error en el send \n");
		            //exit(-1);
		
	            }
	            respuesta = malloc(128);
	            recibidos=recv(socket_subs,respuesta,2,0);
	            if (recibidos<0){
		            close(socket_subs);//cerrar conexión
		            //exit(-1);
	            }
	            respuesta[recibidos]='\0';
	            if(strcmp(respuesta,"ok")!=0){
		            //error en la baja del tema
		            fprintf(stderr,"Evento: Error al dar de baja\n");
		            close(socket_subs);
		            //return 1;
	            }
	            free(respuesta);	
	            size = strlen(valor);
	            if (send(socket_subs,valor,size,0)>=0)
	            {//correcto
		            printf("Evento: Envio: %s %d\n",valor,size);
	            }else{//mal
		            close(socket_subs);//cerrar conexión
		            perror("Evento: Error en el send \n");
		            exit(-1);		
	            }
	            respuesta = malloc(128);
	            recibidos=recv(socket_subs,respuesta,2,0);
	            if (recibidos<0){
		            close(socket_subs);//cerrar conexión
		            exit(-1);
	            }
	            respuesta[recibidos]='\0';
	            if(strcmp(respuesta,"ok")!=0){
		            //error en la baja del tema
		            fprintf(stderr,"Evento: Error al dar de baja\n");
		            close(socket_subs);
		            return 1;
	            }
	            free(respuesta);	
		          close(socket_subs);//cerrar conexión
	            sux = sux->next;
						}
						
					}
					
					
				}else{
					fprintf(stderr,"Evento: Tema no encontrado\n");
					resp=malloc(128);
					strcpy(resp,"error");
					if (send(new_fd,resp,strlen(resp),0)<0)
					{//incorrecto
						perror("Evento: Error send 3\n");
						close(new_fd);
						exit(0);
					}
					free(resp);	
					printf("Evento: Fin conexión con editor\n");
				}
				
			printf("FIN EVENTO -------------\n");
			break;
			case 6: //eliminar subscriptor
			  //por cada tema{
			  printf("\nELIMINAR SUBSCRIPTOR: INICIO -------\n");
			  if(temas.tam==0){
			    break;
			  }
			  eliminado = 1;
			  aux = temas.inicio;
			  while(aux!=NULL){
			    //  borrar_subscriptor(Lista_subs *ls)
			    
			    if(aux->subs != NULL){
			      eliminado = eliminar_sub((aux->subs),atoi(valor));
			      if(eliminado)
			        printf("Eliminar Subscriptor: eliminado de tema %s\n",aux->nombre);
			    }	
			    aux = aux->next;		    
			  }
			  printf("Eliminado de todos los temas\n");
			  //de la lista de subscriptores global borrar subscriptor
			  eliminado = eliminar_sub(&subs,atoi(valor));
			  if(eliminado){
			    printf("Eliminar Subscriptor: eliminado de lista global\n");
			    //enviar ok
			    resp=malloc(128);
					strcpy(resp,"ok");
					if (send(new_fd,resp,strlen(resp),0)<0)
					{//incorrecto
						perror("Error send 3\n");
						close(new_fd);
						exit(0);
					}
					free(resp);	
			  }else{
			    printf("Eliminar Subscriptor: El subscriptor no estaba dado de alta\n");
			    //enviar error
			    resp=malloc(128);
					strcpy(resp,"error");
					if (send(new_fd,resp,strlen(resp),0)<0)
					{//incorrecto
						perror("Error send 3\n");
						close(new_fd);
						exit(0);
					}
					free(resp);	
			  }
			  printf("ELIMINAR SUBSCRIPTOR: FIN\n");
		  break;
		}
		
        /*if (!fork()) { // this is the child process
            close(sock_tcp); // child doesn't need the listener
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }*/
        close(new_fd);  // parent doesn't need this
    }


  
	int i = 0;
	actual = temas.inicio;
	for (i=0;i<temas.tam;i++){
	  
	  fprintf(stdout,"DEBUG: -- Tenemos en la posición(%d): %s\n",i,actual->nombre);
	  actual = actual->next;
	  
	}
 
	
	
	
	fclose(fd);
	return 0;
}

  int buscar_tema(Lista_temas *temas,const char *tema,Tema **vocero){
	  Tema **aux = &(temas->inicio);
	  int encontrado = FALSE;
	
	  while(!encontrado && (*aux)!=NULL){
		  if(strcmp((*aux)->nombre,tema)==0){
			  encontrado = TRUE;	
			  break;		
		  }else{
			  (aux) = &((*aux)->next);
		  }			  			
	  }
	  if(encontrado){
	    if(vocero!=NULL){
	      *vocero = *aux;
	    }
		  
		  return TRUE;
	  }else{
		  return FALSE;
	  }
  }

  int abrir_conexion_tcp_con_servidor(int puerto_servidor, char* direccion_subscriptor)
  {
    int sock_tcp; /* Socket (de tipo TCP) para transmision de datos */
    struct sockaddr_in  dir_tcp_srv;  /* Direccion TCP servidor */   
    /* Creacion del socket TCP */
    fprintf(stdout,"SERVIDOR:  Creacion del socket TCP: ");
    sock_tcp=socket(AF_INET,SOCK_STREAM,0);
    if(sock_tcp<0)
      {
        fprintf(stdout,"ERROR\n");
        exit(1);
      }
    fprintf(stdout,"OK\n");

    /* Nos conectamos con el servidor */
    bzero((char*)&dir_tcp_srv,sizeof(struct sockaddr_in)); /* Pone a 0 
							      la estructura */
    dir_tcp_srv.sin_family=AF_INET;
    dir_tcp_srv.sin_addr.s_addr=inet_addr(direccion_subscriptor);
    dir_tcp_srv.sin_port=htons(puerto_servidor);
    fprintf(stdout,"SERVIDOR:  Conexion al puerto servidor: ");
    if(connect(sock_tcp,(struct sockaddr*)&dir_tcp_srv,sizeof(struct sockaddr_in))<0){
        fprintf(stdout,"ERROR %d\n",errno);
        close(sock_tcp); //exit(1);
      }
    fprintf(stdout,"OK\n");

    return(sock_tcp);
  }

  int baja_subscriptor(Lista_temas *temas,const char* t,int valor){
	  Tema **aux = &(temas->inicio);	
	  int encontrado = FALSE;
	  
	  // Primero buscamos el tema
	  while(!encontrado && *aux!=NULL){
		  if(strcmp(((*aux)->nombre),t)==0){			
			  encontrado = TRUE;			
			  printf("Tema - %s - encontrado\n",(*aux)->nombre);
			  break;
		  }else{
			  aux = &((*aux)->next);			
		  }		
	  }
	  // Ahora buscamos al subscriptor
	  if(encontrado){
	    printf("--------DEBUG: aux %d------------\n",(*aux)->subs->tam);
		  Subscriptor** su = &((*aux)->subs->inicio);
		  Subscriptor** su2 = NULL;
		  int t = 0;
		  encontrado = FALSE;
		  //printf("Llego aquí, liss.tam %d , su: %d\n",(*liss)->tam,(*su)->socket);
		  while(!encontrado && *su != NULL){
		  if((*su)->socket == valor){
			  encontrado = TRUE;	
			  if(t >1){
				  (*su2)->next = (*su)->next;
			  }else{
				  (*aux)->subs->inicio = (*su)->next;
			  }			
			  //free(su);
			  printf("Subscriptor encontrado y eliminado\n");			
		  }else{		
			  su2 = su;
			  su = &((*su)->next);	
			  t++;				
		  }		
	  }
		  if(encontrado){
		    (*aux)->subs->tam--;
			  return TRUE;
		  }else{
			  return FALSE; //EL subscriptor no se encuentra
		  }
			
	  }else{ //El tema no se encuentra
		  return FALSE;
	  }
  }


  void insertar(Lista_temas* temas, char* line){
	    
	    Tema* temita = (Tema*)malloc(sizeof(Tema));
	    temita->nombre = (char *)malloc(strlen(line)+1);
	    strcpy(temita->nombre,line);
	    temita->subs = malloc(sizeof(Lista_subs *));
	    inicializacion_subs(temita->subs);
	    temita->next = NULL;
	    Tema **aux2;
	    
	    if(temas->tam==0){
	      //inicio
	      temas->inicio = temita;			
	    }else{		      
	        aux2= &(temas->inicio);
	        while((*aux2)!=NULL){
	          aux2 = &((*aux2)->next);
	        }
	        *aux2 = temita;	      	  
	    }
	    
	    
	    temas->tam++;
	    //push en la estructura
        printf("Retrieved line of length %d :", temas->tam);
        printf("%s\n", line);
  }

  int crear_socket_TCP(int puerto){
   /* Creacion del socket TCP de servicio */
    struct sockaddr_in sin;
    int socke;
    int puerto_servidor= puerto;
    fprintf(stdout,"Intermediario: socket: ");
    socke = socket(AF_INET, SOCK_STREAM, 0);
    if(socke == -1)
    {
        fprintf(stdout,"ERROR\n");
        return -1;
    }
    
    sin.sin_port = htons(puerto_servidor);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_family = AF_INET;

    fprintf(stdout,"OK\n");  
    fprintf(stdout,"Intermediario: bind: ");
      if(bind(socke, (struct sockaddr *)&sin,sizeof(struct sockaddr_in) ) == -1)
      {
        fprintf(stdout,"ERROR\n");
        close(socke); exit(-1);
      } 
	  fprintf(stdout,"OK\n");

      return socke;
  }
  
  void inicializacion_temas (Lista_temas *lista){
		  lista->inicio = NULL;
		  lista->tam = 0;
		
  }
  void inicializacion_subs (Lista_subs *lista){
		  lista->inicio = NULL;
		  lista-> tam = 0;
  }
  void insertar_sub(Subscriptor *nuevo){
    Subscriptor **aux;
    if(subs.tam==0){
      subs.inicio = nuevo;
      subs.tam = 1;
    }else{
      aux = &(subs.inicio);
      while(*aux != NULL){      
        aux = &((*aux)->next);
      }
      *aux = nuevo;
      subs.tam++;
    }
    
  }
  int eliminar_sub(Lista_subs *ls,int valor){
    Subscriptor **aux,**aux2;
    int t=0;
    int encontrado = 0;
    if(ls!=NULL){
      //Buscar subscriptor y eliminarlo
      aux = &(ls->inicio);
      while(*aux!=NULL && !encontrado){
        if((*aux)->socket==valor){
          encontrado = 1;
          if(t>0){
            (*aux2)->next = (*aux)->next;
            ls->tam--;
            break;
          }else{
            ls->inicio=(*aux)->next;
            ls->tam--;
            break;
          }
        }else{
        aux2=aux;
        aux = &((*aux)->next);                
        t++;
        }
      }
    }
    return encontrado;
  }
  int alta_subscriptor(Lista_subs **subs2, const char* tema,int puerto, char* direccion){
	  //compruebo si subs es null y si lo es añado al inicio
	  int encontrado = FALSE;
	  Subscriptor* aux = malloc(sizeof(Subscriptor*)); 
	  aux->socket = puerto;
	  aux->direccion = malloc(strlen(direccion));
	  aux->direccion = direccion;
	  aux->next = NULL;
	  Subscriptor** aux2;
	  
	  if(*subs2 == NULL){
		  *subs2 = malloc(sizeof(Lista_subs*));
		  inicializacion_subs(*subs2);		
	  }
	  if((*subs2)->tam == 0){
	    (*subs2)->inicio = aux;		
		  (*subs2)->tam++;
		  return FALSE;
	  }else{
	    // Tenemos varios subscriptores
	    aux2 = &((*subs2)->inicio);
	    while(*aux2!=NULL && !encontrado){
	      if((*aux2)->socket == puerto){
	        encontrado = TRUE;
	        break;
	      }
			  aux2 = &((*aux2)->next);			
		  }
		  if(encontrado)return TRUE;
		  *aux2 = aux;// asigno a la última posicion el nuevo nodo	
		  (*subs2)->tam++;	
		  return FALSE;
	    }		
	    
  }
