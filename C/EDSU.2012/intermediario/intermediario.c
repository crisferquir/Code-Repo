#include "comun.h"

#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold

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
  int  insertar_tema(const char* t);
  int   eliminar_tema(const char* t);
  int   abrir_conexion_tcp_con_servidor(int puerto_servidor, char* direccion_subscriptor);

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char *argv[]) 
{
  int sock_tcp, new_fd;/* listen en sock_tcp, nuevas conexiones en new_tcp*/
  int socket_subs;
  FILE *fd;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size = sizeof their_addr;
  char s[INET6_ADDRSTRLEN];  
  char line[100];
  int n;
	Tema* aux = malloc(sizeof(Tema*));
	Msg* mensaje = malloc(sizeof(Msg*));
	Subscriptor *aus = malloc(sizeof(Subscriptor*));
	
	if (argc!=3) {
		fprintf(stderr, "Uso: %s puerto fichero_temas\n", argv[0]);
		return 1;
	}	
	sock_tcp = crear_socket_TCP(atoi(argv[1]));//Crear socket
	fd = fopen(argv[2],"r");			
	while (fgets(line, 100, fd) != NULL){// Incluír en la estructura de datos los temas	  
	  n = (int) strlen (line);
	  line[--n] = '\0';
	  insertar_tema(line);//Crear nuevo tema
  }
  insertar_tema("meta");//tema usado para subscriptores creados
  fclose(fd);
  if (listen(sock_tcp, BACKLOG) == -1) 
        exit(-1);
               
  while(1) {  // main accept() loop
    printf("\nSERVER: waiting for connections...\n");
    new_fd = accept(sock_tcp, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1)
      perror("accept");
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);//direccion del llamante
    printf("server: got connection from %s\n", s);			
		mensaje = recv_msg(new_fd);// Recibir Msg de Editor o subscriptor    
		
		switch(mensaje->op){/* 0:Nuevo_subscriptor, 1:Crear Tema, 2:Eliminar tema, 3:Alta, 4:Baja, 5:Generar evento, 6:Eliminar subscriptor*/
			case 0:
			  printf("\nNUEVO SUBSCRIPTOR --------- \n");			
        if(!insertar_subs("meta", atoi(mensaje->valor),s)){
          enviar_mensaje(new_fd, "er");
          break;
        }
        enviar_mensaje(new_fd, "ok");
        // Enviar cada tema al subscriptor
        aux = inicio;
        while(aux!=NULL){
          if(strcmp(aux->nombre,"meta")!=0){
            socket_subs = abrir_conexion_tcp_con_servidor(atoi(mensaje->valor), s);
            send_msg(socket_subs, aux->nombre,"cebar",0);
            close(socket_subs);
          }
          aux = aux->next;          
        }
			  printf("NUEVO: FIN\n");
			break;
			case 1: /* Crear tema */
			  printf("\nCREAR: INICIO\n");
			  if(insertar_tema(mensaje->tema)){
			    enviar_mensaje(new_fd, "er");
          break;
			  }
        enviar_mensaje(new_fd, "ok");
				close(new_fd);
			  // 2. Avisar a todos los subscriptores 
			  aux = get_tema("meta");
			  if(aux==NULL){
			    break;
			  }			  
			  aus = aux->subs;			  
			  while(aus!=NULL){			    			
          socket_subs = abrir_conexion_tcp_con_servidor(aus->socket, aus->direccion);
          send_msg(socket_subs, mensaje->tema,"crear",0);
          if(!recibir_confirmacion(socket_subs))break;
	        close(socket_subs);	
	        aus = aus->next;        		    
			  }
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
          if(!recibir_confirmacion(socket_subs))break;      
	        close(socket_subs);	
	        aus = aus->next; 
			  }
			  eliminar_tema(mensaje->tema);		
      printf("ELIMINAR: FIN\n");
			break;
	    case 3: // Alta subs
			  printf("\nALTA: INICIO ----------- \n");
        if(!insertar_subs(mensaje->tema,atoi(mensaje->valor),s)){
          enviar_mensaje(new_fd, "er");
          break;
        }
        enviar_mensaje(new_fd,"ok");
				printf("ALTA: FIN ----------- \n");
			break;
	    case 4: // BAJA 
			  printf("\nBAJA: INICIO- --- ----\n");
			  if(!eliminar_subs(mensaje->tema,atoi(mensaje->valor))){
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
		    aux = inicio;
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
		      aux = aux->next;	    
		    }
		    printf("ELIMINAR SUBSCRIPTOR: FIN\n");
		    break;
		}
      close(new_fd);  // parent doesn't need this
    }	
	return 0;
}

/*---------------------------- AUXILIARES ------------------------------------------------*/  

  int abrir_conexion_tcp_con_servidor(int puerto_servidor, char* direccion_subscriptor)
  {
    int sock_tcp; /* Socket (de tipo TCP) para transmision de datos */
    struct sockaddr_in  dir_tcp_srv;  /* Direccion TCP servidor */   

    fprintf(stdout,"SERVIDOR:  Creacion del socket TCP: ");
    sock_tcp=socket(AF_INET,SOCK_STREAM,0);
    if(sock_tcp<0)
      {
        fprintf(stdout,"ERROR\n");
        exit(1);
      }
    fprintf(stdout,"OK\n");
    bzero((char*)&dir_tcp_srv,sizeof(struct sockaddr_in)); /* Pone a 0 la estructura */
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

   int crear_socket_TCP(int puerto)
   {
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
  
  /* -------------------------------------------------------- */
  /* ---- FUNCIONES AUXILIARES PARA EL MANEJO DE LISTAS ----- */
  /* -------------------------------------------------------- */

  Tema* get_tema(const char* t)
  {
    Tema** aux = &inicio;
    int encontrado = 0;
    
    while(*aux!=NULL && !encontrado){
      if(strcmp((*aux)->nombre,t)==0){
        encontrado = 1;
      }else{
        aux = &((*aux)->next);
      }
    }
    return *aux;
  }
  int eliminar_subs(const char* t, int s)
  {
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
      sus = &((*aux)->subs);
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
          (*aux)->subs = (*sus)->next;
          return 1;
        }
      }else{
        return 0;
      }
    }else{
      return 0;
    }    
  }
  
  int insertar_subs(const char* t, int s,char* direccion)
  {
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
      sus = &((*aux)->subs);
      while(*sus!=NULL && !repetido){
        if((*sus)->socket == s){
          repetido = 1;
        }else{
          sus = &((*sus)->next);
        }
      }
      if(repetido){
        return 0;
      }else{
        *sus = malloc(sizeof(Subscriptor*));
        (*sus)->socket = s;
        (*sus)->direccion = direccion;
        (*sus)->next = NULL; 
        return 1;
      }
    }else{
      return 0;
    }
  }

  int insertar_tema(const char* t){
    int existe = 0;
	  Tema *temazo = malloc(sizeof(Tema*));
	  Tema **aux = &inicio;
	  
	  temazo->nombre = malloc(strlen(t));
	  temazo->subs = NULL;
	  temazo->next = NULL;
	  strcpy(temazo->nombre,t);    
    if(*aux == NULL){
      (*aux) = temazo;
      return 0;
    }else{
      while(*aux!=NULL){
        if(strcmp((*aux)->nombre,t)==0){
          existe = 1;
          break;
        }
        aux = &((*aux)->next);        
      }
      if(!existe){
        *aux = temazo;
      }      
      return existe;
    }
  }
  
  int eliminar_tema(const char* t)
  {
    Tema **aux = &inicio;
    Tema **aux2;
    int n = 0;
    int encontrado = 0;
    
    while(!encontrado && *aux!=NULL){
      if(strcmp((*aux)->nombre,t)==0){
        encontrado = 1;
      }else{
        aux2 = aux;        
        aux = &((*aux)->next);        
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
