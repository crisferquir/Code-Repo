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


typedef struct mensaje_struct{
  char* tema;
  char* valor;
  int op;
}Msg;
// Lista subscriptores
typedef struct suscriptor_Struct{
  int socket;
  char* direccion;
  struct suscriptor_Struct* next;
}Subscriptor;

typedef struct tema_Struct{
  char* nombre; 
  Subscriptor *subs;  
  struct tema_Struct *next;
}Tema;

Tema* inicio;


  int crear_socket_TCP(int puerto);

  Tema* get_tema(const char* t);
  int   eliminar_subs(const char* t, int s);
  int   insertar_subs(const char* t, int s, char* direccion);
  void  insertar_tema(const char* t);
  int   eliminar_tema(const char* t);

  int   recibir_confirmacion(int puerto);
  char* recibir_mensaje     (int puerto);
  int   enviar_mensaje      (int puerto, char* mensaje);
  Msg*  recv_msg            (int puerto);
  int   send_msg            (int puerto, char* tema,char* valor,int op);

  int   baja_subscriptor    (Lista_temas *temas,const char* tema,int valor);
  int   abrir_conexion_tcp_con_servidor(int puerto_servidor, char* direccion_subscriptor);

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


int main(int argc, char *argv[]) {
  int sock_tcp, new_fd;/* listen en sock_tcp, nuevas conexiones en new_tcp*/
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  char s[INET6_ADDRSTRLEN];
  FILE *fd;
  char line[100];
  int n;
  Tema* actual = malloc(sizeof(Tema*));
	Tema* aux = malloc(sizeof(Tema*));
	Msg* mensaje = malloc(sizeof(Msg*));
	
	if (argc!=3) {
		fprintf(stderr, "Uso: %s puerto fichero_temas\n", argv[0]);
		return 1;
	}
	//Crear socket
	sock_tcp = crear_socket_TCP(atoi(argv[1]));
	fd = fopen(argv[2],"r+");		
	// Incluír en la estructura de datos los temas
	while (fgets(line, 100, fd) != NULL){
	  //Crear nuevo tema
	  n = (int) strlen (line);
	  line[--n] = '\0';
	  insertar_tema(line);
  }
  fclose(fd);
  if (listen(sock_tcp, BACKLOG) == -1) {
        exit(-1);
  }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        exit(-1);
    }
       
    while(1) {  // main accept() loop
    printf("\nSERVER: waiting for connections...\n");
    sin_size = sizeof their_addr;
    new_fd = accept(sock_tcp, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
    }
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
    printf("server: got connection from %s\n", s);
    
		char buf[128] = "";
		char tema[128];
		char valor[128];
		int op;
		int recived;
		char *resp;
		
		// Recibir Msg de Editor o subscriptor
		mensaje = recv_msg(new_fd);
    
		
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
		switch(mensaje->op){/* 0:Nuevo_subscriptor, 1:Crear Tema, 2:Eliminar tema, 3:Alta, 4:Baja, 5:Generar evento*/
			case 0: 
			  printf("\nNUEVO SUBSCRIPTOR --------- \n");			
        if(!insertar_subs("meta", atoi(mensaje->valor),s){
          enviar_mensaje(new_fd, "er");
          break;
        }
        enviar_mensaje(new_fd, "ok");
        // Enviar cada tema al subscriptor
        aux = inicio;
        while(aux!=NULL){
          socket_subs = abrir_conexion_tcp_con_servidor(atoi(mensaje->puerto), s);
          send_msg(socket_subs, aux->nombre,"cebar",0);
          close(socket_subs);
          if(aux->next == NULL){
            socket_subs = abrir_conexion_tcp_con_servidor(nuevo->socket, nuevo->direccion);
            send_msg(socket_subs, aux->nombre,"aa",0);
            close(socket_subs);
          }
          aux = aux->next;
          close(socket_subs);//cerrar conexión
        }
			  printf("Nuevo: Todos los temas enviados al subscriptor\n");
			  printf("NUEVO: FIN\n");
			break;
			case 1: /* Crear tema */
			  printf("\nCREAR: INICIO\n");
			  if(!insertar_tema(mensaje->tema)){
			    enviar_mensaje(new_fd, "er");
          break;
			  }
        enviar_mensaje(new_fd, "ok");
				close(new_fd);
			  printf("Crear: Fin conexión con editor\n");
			  // 2. Avisar a todos los subscriptores 			  
			  aus = get_tema("meta")->subs;			  
			  while(aus!=NULL){
			    //Conectar al subscriptor			
          socket_subs = abrir_conexion_tcp_con_servidor(aus->socket, aus->direccion);
          send_msg(socket_subs, mensaje->tema,"crear",0);
	        close(socket_subs);	
	        aus = aus->next;        		    
			  }
			  printf("Crear: Fin conexion subscriptores\n");
			  printf("FIN CREAR\n");
			break;
			
			case 2: /* Eliminar tema */
			printf("ELIMINAR: INICIO\n");
			  aux = get_tema(mensaje->tema);
			  if(aux == NULL){
			    enviar_mensaje(new_fd, "er");
          break;
			  }
			  enviar_mensaje(new_fd,"ok");
			  close(new_fd);
			  aus = aux->subs;
			  while(aus!=NULL){
			    socket_subs = abrir_conexion_tcp_con_servidor(aus->socket, aus->direccion);
          send_msg(socket_subs, mensaje->tema,"eliminar",0);          
	        close(socket_subs);	
	        aus = aus->next; 
			  }
			  eliminar_tema(mensaje->tema);		
      printf("ELIMINAR: FIN\n");
			break;
	case 3: // Alta subs
				t=0;
				encontrado = 0;
			  printf("\nALTA: INICIO ----------- \n");
        if(!insertar_subs(mensaje->tema,mensaje->valor,s)){
          enviar_mensaje(new_fd, "er");
          break;
        }
        enviar_mensaje(new_fd,"ok");
				printf("ALTA: FIN ----------- \n");
			break;
			case 4: // BAJA 
			  printf("\nBAJA: INICIO- --- ----\n");
			  if(!eliminar_subs(mensaje->tema,mensaje->valor)){
          enviar_mensaje(new_fd, "er");
          break;
        }
        enviar_mensaje(new_fd,"ok");
				printf("FIN: BAJA ---------------\n");
			break;
			
	case 5: //Generar Evento
	  aux = get_tema(mensaje->tema);
	  if(aux==NULL){
	    enviar_mensaje(new_fd, "er");
      break;
	  }
	  enviar_mensaje(new_fd,"ok");
	  close(new_fd);
	  aus = aux->subs;
	  while(aus!=NULL){
	    socket_subs = abrir_conexion_tcp_con_servidor(aus->socket, aus->direccion);
      send_msg(socket_subs, mensaje->tema,mensaje->valor,0);          
	    close(socket_subs);	
	    aus = aus->next;
	  }				
		printf("FIN EVENTO -------------\n");
		break;
		
	case 6: //eliminar subscriptor
	  printf("\nELIMINAR SUBSCRIPTOR: INICIO -------\n");
		aux = &inicio;
		while(aux!=NULL){
	    if(strcmp(aux->nombre,"meta") == 0){
		    if(!eliminar_subs(aux->nombre,atoi(mensaje->valor))){
			    enviar_mensaje(new_fd, "er");
          break;
			  }
			  enviar_mensaje(new_fd, "ok");
		  }else{
			  eliminar_subs(aux->nombre,atoi(mensaje->valor));
		  }			    
		}
		printf("ELIMINAR SUBSCRIPTOR: FIN\n");
		break;
		}
      close(new_fd);  // parent doesn't need this
    }	
	return 0;
}

/*
  FUNCIONES AUXILIARES MANEJO DE MENSAJES Y CONEXIONES  
*/

  int send_msg(int puerto, char* tema,char* valor,int op){
    char operacion[10] = itoa(op,str,10);
    if(!enviar_mensaje(puerto,tema)){return 0;}
    if(!recibir_confirmacion(puerto)){return 0;}
    if(!enviar_mensaje(puerto,valor)){return 0;}
    if(!recibir_confirmacion(puerto)){return 0;}
    if(!enviar_mensaje(puerto,operacion)){return 0;}
    if(!recibir_confirmacion(puerto)){return 0;}
    
  }
  Msg* recv_msg(int puerto){
    Msg* mensaje = malloc(sizeof(Msg*));
    mensaje->tema = recibir_mensaje(puerto);
    if(!recibir_confirmacion(puerto)){return NULL;}
    mensaje->valor = recibir_mensaje(puerto);
    if(!recibir_confirmacion(puerto)){return NULL;}
    mensaje->op = atoi(recibir_mensaje(puerto));
    if(!recibir_confirmacion(puerto)){return NULL;}
        
    return mensaje;    
  }

  int enviar_mensaje(int puerto, char* mensaje){
    int result = 0;
    if ((result = send(puerto,mensaje,strlen(mensaje),0))<0){
						perror("Error send 3\n");
						close(new_fd);
		}
		return result;		
  }
  
  char* recibir_mensaje(int puerto){
    char recibidos[128];    
	  if (recv(puerto,recibidos,128,0)<0){
		  close(socket_subs);//cerrar conexión
    }
	  respuesta[recibidos]='\0';  
	  return recibidos; 
  }
  
  int recibir_confirmacion(int puerto){
    char recibidos[128];
    
	  if(recv(puerto,recibidos,2,0)<0){
		   close(socket_subs);//cerrar conexión
	  }
	  respuesta[recibidos]='\0';  
	  if(strcmp(recibidos,"ok")==0){
	    return 1;
	  }else{
	    return 0;
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

  
  /* -------------------------------------------------------- */
  /* ---- FUNCIONES AUXILIARES PARA EL MANEJO DE LISTAS ----- */
  /* -------------------------------------------------------- */

  Tema* get_tema(const char* t){
    Tema** aux = &inicio;
    int encontrado = 0;
    while(*aux!=NULL && !encontrado){
      if(strcmp((*aux)->nombre,t)==0){
        encontrado = 1;
      }else{
        aux = (*aux)->next;
      }
    }
    return *aux;
  }
  int eliminar_subs(const char* t, int s){
    Tema **aux =&inicio;
    Subscriptor **sus,**sus2;
    int encontrado = 0;
    while(*aux!=NULL && !encontrado){
      if(strcmp((*aux)->nombre,t)==0){
        encontrado = 1;
      }else{
        aux = &((*aux)->next);
      }      
    }
    if(encontrado){
      sus = (*aux)->subs;
      encontrado = 0;
      while(*sus != NULL && !encontrado){
        if((*sus)->socket == s){
          encontrado = 1;
        }else{
          sus2 = sus;
          sus = &((*sus)->next);
        }
      }
      if(encontrado){
        if((*aux)->subs->socket != s){
          (*sus2)->next = (*sus)->next;
          return 1;
        }else{
          (*aux)->subs = (*aux)->next;
          return 0;
        }
      }else{
        return 0;
      }
    }else{
      return 0;
    }    
  }
  
  int insertar_subs(const char* t, int s,char* direccion){
    Tema **aux = &inicio;
    Subscriptor **sus;
    int encontrado = 0;
    int repetido = 0;
    while(*aux!=NULL && !encontrado){
      if(strcmp((*aux)->nombre,t)==0){
        encontrado = 1;
      }else{
        aux = &((*aux)->next);
      }    
    }
    if(encontrado){
      sus = (*aux)->subs;
      while(*sus!=NULL && !repetido){
        if((*sux)->socket == s){
          repetido = 1;
        }else{
          sus = &((*sus)->next);
        }
      }
      if(repetido){
        return 0;
      }else{
        *sus = malloc(sizeof(*Subscriptor))
        (*sus)->socket = s;
        (*sux)->direccion = direccion;
        (*sus)->next = NULL; 
        return 1;
      }
    }else{
      return 0;
    }
  }

  void insertar_tema(const char* t){
	  Tema *temazo = malloc(sizeof(Tema*));
	  temazo->nombre = malloc(strlen(t));
	  temazo->subs = NULL;
	  temazo->next = NULL;
	  strcpy(temazo->nombre,t);

    Tema **aux = &inicio;
    if(*aux == NULL){
      printf("LISTA: inicio lista %s\n",temazo->nombre);
      (*aux) = temazo;
    }else{
      while(*aux!=NULL){
        aux = &(*aux)->next;
      }
      *aux = temazo;
      printf("LISTA: anadir tema %s\n",temazo->nombre);
    }
  }
  
  int eliminar_tema(const char* t){
    Tema **aux = &inicio;
    Tema **aux2;
    int n = 0;
    int encontrado = 0;
    while(!encontrado && *aux!=NULL){
      if(strcmp((*aux)->nombre,t)==0){
        encontrado = 1;
      }else{
        if(n!=0){
          aux2 = aux;
        }
        aux = &(*aux)->next;        
        n++;
      }
    }
    if(encontrado){
      if(n!=0){
        (*aux2)->next = (*aux)->next; 
      }else{
        inicio = (*aux)->next;
      }
      return 1;    
    }else{
      return 0;
    }
  }
