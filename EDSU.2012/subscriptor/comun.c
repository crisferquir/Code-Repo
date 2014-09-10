/*
   Incluya en este fichero todas las implementaciones que pueden
   necesitar compartir todos los módulos (editor, subscriptor y
   proceso intermediario), si es que las hubiera.
*/

#include "comun.h"

/*
  FUNCIONES AUXILIARES DE INT A STRING
*/
/* itoa:  convert n to characters in s */
 void itoa(int n, char s[])
 {
     int i, sign;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }
  /* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }
/*
  FUNCIONES AUXILIARES MANEJO DE MENSAJES Y CONEXIONES  
*/

  int send_msg(int puerto, char* tema,char* valor,int op){
    char operacion[10];
    itoa(op,operacion);
    if(!enviar_mensaje(puerto,tema)){return 0;}
    if(!recibir_confirmacion(puerto)){return 0;}
    if(!enviar_mensaje(puerto,valor)){return 0;}
    if(!recibir_confirmacion(puerto)){return 0;}
    if(!enviar_mensaje(puerto,operacion)){return 0;}
    if(!recibir_confirmacion(puerto)){return 0;}
    return 1;
  }  
  Msg* recv_msg(int puerto){
    Msg* mensaje = malloc(sizeof(Msg*));
    char *n = malloc(10);
    char *tema = malloc(128);
    char *valor = malloc(128);
    
    recibir_mensaje(puerto,tema );
    if(!enviar_mensaje(puerto,"ok")){return NULL;}
    recibir_mensaje(puerto,valor);
    if(!enviar_mensaje(puerto,"ok")){return NULL;}
    recibir_mensaje(puerto,n);
    mensaje->op = atoi(n) ;
    if(!enviar_mensaje(puerto,"ok")){return NULL;}
        
    mensaje->tema = malloc(strlen(tema));
    strcpy(mensaje->tema,tema);
    mensaje->valor = malloc(strlen(valor));
    strcpy(mensaje->valor,valor);
    
    return mensaje;    
  }

  int enviar_mensaje(int puerto, char* mensaje){
    int result = 0;
    char* m = malloc(128);
    strcpy(m,mensaje);
    if ((result = send(puerto,m,strlen(m),0))<0){
						perror("Error send 3\n");
						close(puerto);
		}
		return result;		
  }
  
  void recibir_mensaje(int puerto,char* recibidos){ 
    int c;   
    char* m = malloc(128);
	  if ((c=recv(puerto,m,128,0))<0){
		  close(puerto);//cerrar conexión
    }
	  m[c]='\0';   
	  strcpy(recibidos,m);
  }
  
  int recibir_confirmacion(int puerto){
    char recibidos[128];
    int c;
	  if((c = recv(puerto,recibidos,2,0))<0){
		   close(puerto);//cerrar conexión
	  }
	  recibidos[c]='\0';  
	  if(strcmp(recibidos,"ok")==0){
	    return 1;
	  }else{
	    return 0;
	  }
  }
