#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include "subscriptor.h"

static int num_eventos;
static sem_t sem;

static void notificacion_evento(const char *t, const char *e){
        printf("\n-> Recibido evento %s con valor %s\n", t, e);
	if (--num_eventos==0)
		sem_post(&sem);
}

int main(int argc, char *argv[]) {
	int i;

	if ((argc<3) || ((num_eventos=atoi(argv[1]))<1)) {
                fprintf(stderr, "Uso: %s num_eventos tema...\n", argv[0]);
                return 1;
        }
	sem_init(&sem, 0, 0);

	inicio_subscriptor(notificacion_evento, NULL, NULL);
	for (i=2; i<argc; i++) 
		if (alta_subscripcion_tema(argv[i])<0) 
                	fprintf(stderr, "Error en subscripción a tema %s\n",
				 argv[i]);

	sem_wait(&sem);

	for (i=2; i<argc; i++) 
		if (baja_subscripcion_tema(argv[i])<0)
                	fprintf(stderr, "Error baja subscripción a tema %s\n",
				 argv[i]);

	return 0;
}
