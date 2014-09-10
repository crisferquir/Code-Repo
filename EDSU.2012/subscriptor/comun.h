/*
   Incluya en este fichero todas las definiciones que pueden
   necesitar compartir todos los módulos (editor, subscriptor y
   proceso intermediario), si es que las hubiera.
*/
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
  int   recibir_confirmacion(int puerto);
  void recibir_mensaje      (int puerto,char* recibidos);
  int   enviar_mensaje      (int puerto, char* mensaje);
  Msg*  recv_msg            (int puerto);
  int   send_msg            (int puerto, char* tema,char* valor,int op);

  void itoa(int n, char s[]);
  void reverse(char s[]);
