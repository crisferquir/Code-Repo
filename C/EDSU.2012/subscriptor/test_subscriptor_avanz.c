#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include "subscriptor.h"

static int num_eventos;
static sem_t sem;

static void alta_tema(const char *t){
        printf("\n-> Recibida alta tema %s\n", t);
	if (alta_subscripcion_tema(t)<0) 
               	fprintf(stderr, "Error en subscripción a tema %s\n", t);
}
static void baja_tema(const char *t){
        printf("\n-> Recibida baja tema %s\n", t);
}

static void notificacion_evento(const char *t, const char *e){
        printf("\n-> Recibido evento %s con valor %s\n", t, e);
	if (--num_eventos==0)
		sem_post(&sem);
}

int main(int argc, char *argv[]) {

	if ((argc!=2) || ((num_eventos=atoi(argv[1]))<1)) {
                fprintf(stderr, "Uso: %s num_eventos\n", argv[0]);
                return 1;
        }
	sem_init(&sem, 0, 0);
	inicio_subscriptor(notificacion_evento, alta_tema, baja_tema);

	sem_wait(&sem);
	fin_subscriptor();

	return 0;
}
