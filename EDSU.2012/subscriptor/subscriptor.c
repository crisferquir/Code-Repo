#include "comun.h"
#include "edsu_comun.h"

#define BACKLOG 10 
  /* -------------DECLARACIÓN DE VARIABLES GLOBALES -----*/
  int puerto,socket_fd; // para conectarnos al intermediario

  /* -------------TIPOS Y ESTRUCTURAS DE DATOS ----------*/
  typedef struct tema_Struct{
    char* nombre; 
    struct tema_Struct *next;
  }Tema;
  Tema *inicio;
  typedef struct my_thread_struct{
    pthread_t thread;
    struct my_thread_struct* next;
  }My_thread;
  My_thread *threads;

  // Funciones auxilares
  int crear_socket_TCP();
  int abrir_conexion_tcp_con_servidor(int puerto_servidor, char* direccion_servidor);
  void (*notificacion)(const char *, const char *);//funcion de notificacion
  void (*altaTema)(const char *);
  void (*bajaTema)(const char *);
  void insertar_tema(const char* t);
  void eliminar_tema(const char* t);
  void crear_thread();
  // get sockaddr, IPv4 or IPv6:
  void *get_in_addr(struct sockaddr *sa)
  {
      if (sa->sa_family == AF_INET) {
          return &(((struct sockaddr_in*)sa)->sin_addr);
      }
      return &(((struct sockaddr_in6*)sa)->sin6_addr);
  }
  int pendientes = 0;
  Msg* t_mensaje;
  pthread_mutex_t lock;
  volatile int holder = 0;
  /* -------------------------------------------------------- */
  /* ---------------- FUNCIONES PRINCIPALES ----------------- */
  /* -------------------------------------------------------- */  
  int alta_subscripcion_tema(const char *tema) 
  {
	  char* direccion_servidor = getenv("SERVIDOR");
	  int puerto_servidor= atoi(getenv("PUERTO"));
	  int socket_tcp;
	  char *mensaje = malloc(10);
	  
	  itoa(puerto,mensaje);
	  socket_tcp = abrir_conexion_tcp_con_servidor(puerto_servidor,direccion_servidor);	  
	  printf("ALTA SUBS-------------\n");
	  if(!send_msg(socket_tcp,(char *)tema,mensaje,3)){
	    close(socket_tcp);
	    return -1;
	  }
	  if(!recibir_confirmacion(socket_tcp)){
	    printf("ALTA generado ERROR\n");
	    close(socket_tcp);
	    return -1;
	  }else{
	    printf("ALTA generado CORRECTAMENTE\n");
	    close(socket_tcp);
	    return 0;
	  }
  }

  int baja_subscripcion_tema(const char *tema) 
  {
	  char* direccion_servidor = getenv("SERVIDOR");
	  int puerto_servidor= atoi(getenv("PUERTO"));
	  int socket_tcp;
	  char *mensaje = malloc(10);
	  
	  itoa(puerto,mensaje);	  
	  socket_tcp = abrir_conexion_tcp_con_servidor(puerto_servidor,direccion_servidor);
	  printf("BAJA SUBS-------------\n");
	  if(!send_msg(socket_tcp,(char *)tema,mensaje,4)){
	    close(socket_tcp);
	    return -1;
	  }
	  if(!recibir_confirmacion(socket_tcp)){
	    printf("BAJA generado ERROR\n");
	    close(socket_tcp);
	    return -1;
	  }else{
	    printf("BAJA generado CORRECTAMENTE\n");
	    close(socket_tcp);
	    return 0;
	  }
  }
  
   void* j_function(void *socket)
   {
      char* tema = malloc(128);
      char* valor = malloc(128);
      int operacion;
      while(holder)
        pthread_mutex_lock(&lock);//mutex lock
      strcpy(tema,t_mensaje->tema);
      strcpy(valor,t_mensaje->valor);
      operacion = t_mensaje->op;
      holder = 1;
      
	    /* Leemos el mensaje del intermediario */
	    if(strcmp((char*)valor,"crear")==0){
	      printf("CREAR TEMAS_%s\n",(char *)tema);
	      altaTema((char*)tema);	      
	    }else if(strcmp((char*)valor,"eliminar")==0){
	      printf("ELIMINAR TEMAS_%s\n",(char*)tema);
	      bajaTema((char*)tema);	     
	    }else if(strcmp((char*)valor,"cebar")==0){
	      printf("CEBAR TEMAS_%s\n",(char*)tema);
	      altaTema((char*)tema);	   
	    }else{
	      notificacion((char*)tema,(char*)valor);    
	    }   
	    pthread_mutex_unlock(&lock);//mutex unlock
	  return NULL;
   }

  void* t_function(void *socket)
  {
	  int sock_tcp = socket_fd;
	  int new_fd;
	  struct sockaddr_storage their_addr;
	  socklen_t tam;	
	  struct sockaddr_in dir_tcp_cli;
	  tam = sizeof(dir_tcp_cli);
	  socklen_t sin_size = sizeof their_addr;
	  char s[INET6_ADDRSTRLEN];
    Msg* mensaje = (Msg*)malloc(sizeof(Msg*));
    
    t_mensaje = (Msg*)malloc(sizeof(Msg*));
    t_mensaje->tema = malloc(128);
    t_mensaje->valor = malloc(128);
    holder = 1;
	  while(1){  
	    printf("subscriptor: waiting for connections en el puerto: %d\n",(int)puerto);	      
      new_fd = accept(sock_tcp, (struct sockaddr *)&their_addr, &sin_size);
      if (new_fd == -1) 
        perror("accept");
		  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
      printf("subscriptor: got connection from %s\n", s);          
      mensaje = recv_msg(new_fd);
      close(new_fd);
      while(!holder)
        pthread_mutex_lock(&lock);//mutex lock    
      strcpy(t_mensaje->tema,mensaje->tema);
      strcpy(t_mensaje->valor,mensaje->valor);
      t_mensaje->op = mensaje->op;
      holder = 0;
      pthread_mutex_unlock(&lock);//mutex unlock
      crear_thread();	    
  }
}
// Lista auxiliar
  void crear_thread()
  {
    My_thread** aux = &threads;
    My_thread* j = malloc(sizeof(My_thread*));
    pthread_create(&(j->thread),NULL,&j_function, NULL);/* Se encarga de lanzar las funciones */
    while(*aux!=NULL){
      aux = &((*aux)->next);
    }
    *aux = j;
  }
  /* para la version inicial solo se usa el primer argumento dejando los restantes a NULL */
  int inicio_subscriptor(void (*notif_evento)(const char *, const char *),
                  void (*alta_tema)(const char *),
                  void (*baja_tema)(const char *)) 
  {
    char* direccion_servidor = getenv("SERVIDOR");
	  int puerto_servidor= atoi(getenv("PUERTO"));
	  int socket_tcp;
	  char *mensaje = malloc(10);
	  pthread_t t;
	  	  
	  if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return -1;
    }
	  notificacion = notif_evento; // funcion guardada	  
	  crear_socket_TCP();/* Creación socket de eventos */
	  itoa(puerto,mensaje);	  
	  pthread_create(&t,NULL,&t_function, NULL);/* Queda a la espera de notificaciones */
	  if(alta_tema!=NULL){ //avisar al servidor op:0 Nuevo Subscriptor 
	    altaTema = alta_tema;	    
	    socket_tcp = abrir_conexion_tcp_con_servidor(puerto_servidor,direccion_servidor);
	    // construír mensaje
	    printf("NUEVO SUBS-------------\n");
	    if(!send_msg(socket_tcp,"nada",mensaje,0))
	      return -1;
	    if(!recibir_confirmacion(socket_tcp))
	        return -1;
	    else
	        printf("NUEVO generado CORRECTAMENTE\n");
	    close(socket_tcp);    
	  }
	  if(baja_tema!=NULL)
	    bajaTema = baja_tema;			
	  return 0;
  }

int fin_subscriptor() 
{
	  char* direccion_servidor = getenv("SERVIDOR");
	  int puerto_servidor= atoi(getenv("PUERTO"));
	  int socket_tcp;
	  char *mensaje = malloc(10);
	  
	  itoa(puerto,mensaje);	  
	  socket_tcp = abrir_conexion_tcp_con_servidor(puerto_servidor,direccion_servidor);
	  printf("ELIMINAR SUBS-------------\n");
	  if(!send_msg(socket_tcp,"nada",mensaje,6)){
	    close(socket_tcp);
	    return -1;
	  }
	  if(!recibir_confirmacion(socket_tcp)){
	    printf("ELIMINAR generado ERROR\n");
	    close(socket_tcp);
	    return -1;
	  }else{
	    printf("ELIMINAR generado CORRECTAMENTE\n");
	    close(socket_tcp);
	    return 0;
	  }
}

  /* -------------------------------------------------------- */
  /* ---------------- FUNCIONES AUXILIARES ------------------ */
  /* -------------------------------------------------------- */  
  /* ------ FUNCIONES RELACIONADAS CON CONEXIONES ----------- */
  /* -------------------------------------------------------- */
  int crear_socket_TCP()
  {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    int socke;
    
    fprintf(stdout,"Intermediario: socket: ");
    socke = socket(AF_INET, SOCK_STREAM, 0);
    if(socke == -1)
    {
        fprintf(stdout,"ERROR\n");
        return -1;
    }    
    sin.sin_port = 0;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_family = AF_INET;
    fprintf(stdout,"OK\n");  
    fprintf(stdout,"Intermediario: bind: ");
    if(bind(socke, (struct sockaddr *)&sin, len) == -1)
    {
      fprintf(stdout,"ERROR\n");
      close(socke); return(-1);
    } 
	  fprintf(stdout,"OK\n");    
	  if (listen(socke, BACKLOG) == -1) {
          perror("listen");
          return(-1);
    }	    
    if (getsockname(socke, (struct sockaddr *)&sin, &len) == -1)
        perror("getsockname");    
	  puerto = ntohs(sin.sin_port);
	  socket_fd = socke;
    printf("socke: %d\n",socke);
     return socke;
}    
   /* Funcion que abre un nuevo socket TCP para la transmision de datos */
  int abrir_conexion_tcp_con_servidor(int puerto_servidor, char* direccion_servidor)
  {
    int sock_tcp; /* Socket (de tipo TCP) para transmision de datos */
    struct sockaddr_in  dir_tcp_srv;  /* Direccion TCP servidor */ 
    struct hostent *host_info;
      
    /* Creacion del socket TCP */
    fprintf(stdout,"CLIENTE:  Creacion del socket TCP: ");
    sock_tcp=socket(AF_INET,SOCK_STREAM,0);
    if(sock_tcp<0)
      {
        fprintf(stdout,"ERROR\n");
        return(1);
      }
    fprintf(stdout,"OK\n");
    /* Nos conectamos con el servidor */
    bzero((char*)&dir_tcp_srv,sizeof(struct sockaddr_in)); /* Pone a 0 la estructura */    
    host_info=gethostbyname(direccion_servidor); 
    dir_tcp_srv.sin_family=AF_INET;
    memcpy(&dir_tcp_srv.sin_addr.s_addr, host_info->h_addr, host_info->h_length);
    dir_tcp_srv.sin_port=htons(puerto_servidor);
    fprintf(stdout,"CLIENTE:  Conexion al puerto servidor: ");
    if(connect(sock_tcp,(struct sockaddr*)&dir_tcp_srv,sizeof(struct sockaddr_in))<0){
        fprintf(stdout,"ERROR %d\n",errno);
        close(sock_tcp); return(1);
      }
    fprintf(stdout,"OK\n");

    return(sock_tcp);
  }


  
  /* -------------------------------------------------------- */
  /* ---- FUNCIONES AUXILIARES PARA EL MANEJO DE LISTAS ----- */
  /* -------------------------------------------------------- */

  void insertar_tema(const char* t)
  {
	  Tema *temazo = malloc(sizeof(Tema*));
	  Tema **aux = &inicio;
	  
	  temazo->nombre = malloc(strlen(t));
	  temazo->next = NULL;
	  strcpy(temazo->nombre,t);    
    if(*aux == NULL){
      printf("LISTA: inicio lista %s\n",temazo->nombre);
      (*aux) = temazo;
    }else{
      while(*aux!=NULL)
        aux = &((*aux)->next);      
      *aux = temazo;
      printf("LISTA: anadir tema %s\n",temazo->nombre);
    }
  }
  
  void eliminar_tema(const char* t)
  {
    Tema **aux = &inicio;
    Tema **aux2;
    int n = 0;
    int encontrado = 0;
    
    while(!encontrado && *aux!=NULL){
      if(strcmp((*aux)->nombre,t)==0){
        encontrado = 1;
        break;
      }else{       
        aux2 = aux;
        aux = &((*aux)->next);        
        n++;
      }
    }
    printf("Eliminando tema %s\n",(*aux)->nombre);
    if(n!=0)
      (*aux2)->next = (*aux)->next; 
    else
      inicio = (*aux)->next;
  }
