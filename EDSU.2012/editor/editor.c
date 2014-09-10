  #
  #include "comun.h"
  #include "edsu_comun.h"

    //Auxiliares
  int abrir_conexion_tcp_con_servidor(int puerto_servidor,char* direccion_servidor); 
    
  int generar_evento(const char *tema, const char *valor) 
  {  
	  char* direccion_servidor = getenv("SERVIDOR");
	  int puerto_servidor= atoi(getenv("PUERTO"));
	  int socket_tcp;
	
	  socket_tcp = abrir_conexion_tcp_con_servidor(puerto_servidor,direccion_servidor);
	  printf("GENERAR EVENTO-------------\n");
	  if(!send_msg(socket_tcp,(char *)tema,(char *)valor,5)){
	    close(socket_tcp);
	    return -1;
	  }
	  if(!recibir_confirmacion(socket_tcp)){
	    close(socket_tcp);
	    printf("Evento generado ERROR\n");
	    return -1;
	  }else{
	    close(socket_tcp);
	    printf("Evento generado CORRECTAMENTE\n");
	    return 0;
	  }	  
  }

  int crear_tema(const char *tema) 
  {  
	  char* direccion_servidor = getenv("SERVIDOR");
	  int puerto_servidor= atoi(getenv("PUERTO"));
	  int socket_tcp;
	
	  socket_tcp = abrir_conexion_tcp_con_servidor(puerto_servidor,direccion_servidor);
	  // construír mensaje
	  printf("CREAR TEMA-------------\n");
	  if(!send_msg(socket_tcp,(char *)tema,"cualquiera",1)){
	    close(socket_tcp);
	    return -1;
	  }
	  if(!recibir_confirmacion(socket_tcp)){
	    printf("CREAR generado ERROR\n");
	    close(socket_tcp);
	    return -1;
	  }else{
	    printf("CREAR generado CORRECTAMENTE\n");
	    close(socket_tcp);
	    return 0;
	  }
  }

  int eliminar_tema(const char *tema) 
  {
	  char* direccion_servidor = getenv("SERVIDOR");
	  int puerto_servidor= atoi(getenv("PUERTO"));
	  int socket_tcp;
	
	  socket_tcp = abrir_conexion_tcp_con_servidor(puerto_servidor,direccion_servidor);
	  // construír mensaje
	  printf("ELIMINAR TEMA-------------\n");
	  if(!send_msg(socket_tcp,(char*)tema,"cualquiera",2)){
	    close(socket_tcp);
	    return -1;
	  }
	  if(!recibir_confirmacion(socket_tcp)){
	    close(socket_tcp);
	    printf("ELIMINAR generado ERROR\n");
	    return -1;
	  }else{
	    close(socket_tcp);
	    printf("ELIMINAR generado CORRECTAMENTE\n");
	    return 0;
	  }
  }

    /* -------------------------------------------------------- */
    /* ---- FUNCIONES AUXILIARES PARA EL MANEJO DE CONEXI ----- */
    /* -------------------------------------------------------- */
    
    int abrir_conexion_tcp_con_servidor(int puerto_servidor, char* direccion_servidor)
    {
      int sock_tcp; /* Socket (de tipo TCP) para transmision de datos */
      struct sockaddr_in  dir_tcp_srv;  /* Direccion TCP servidor */ 
      struct hostent *host_info;
        
      sock_tcp=socket(AF_INET,SOCK_STREAM,0);
      if(sock_tcp<0)
        {
          return -1;
        }
      /* Nos conectamos con el servidor */
      bzero((char*)&dir_tcp_srv,sizeof(struct sockaddr_in)); /* Pone a 0 la estructura */      
      host_info=gethostbyname(direccion_servidor); 
      dir_tcp_srv.sin_family=AF_INET;
      memcpy(&dir_tcp_srv.sin_addr.s_addr, host_info->h_addr, host_info->h_length);
      dir_tcp_srv.sin_port=htons(puerto_servidor);
      if(connect(sock_tcp,(struct sockaddr*)&dir_tcp_srv,sizeof(struct sockaddr_in))<0){
          close(sock_tcp); return -1;
        }
      return(sock_tcp);
    }  
