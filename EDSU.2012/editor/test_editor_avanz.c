#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "editor.h"


int main(int argc, char *argv[]) {
	int i;
	int num_eventos;
	int num_temas;
	char **temas;
	char tema[16];
	char valor[16];

        if ((argc!=3) || ((num_eventos=atoi(argv[1]))<1) || ((num_temas=atoi(argv[2]))<1) ) {
                fprintf(stderr, "Uso: %s num_eventos num_temas\n", argv[0]);
                return 1;
        }

	temas = malloc(num_temas * sizeof(char *));
	for (i=0; i<num_temas; i++) {
		sprintf(tema, "tema%d", i);
		temas[i] =  strdup(tema);
		crear_tema(temas[i]);
	}
	printf("Temas creados; pulse para continuar: ");
	getchar();

	for (i=0; i<num_eventos; i++) {
		sprintf(valor, "%d", i);
		if (generar_evento(temas[i%num_temas], valor)<0)
			fprintf(stderr, "Error generando evento para tema %s\n",
                                 temas[i%num_temas]);
	}
	printf("Eventos generados; pulse para continuar: ");
	getchar();

	for (i=0; i<num_temas; i++) {
		eliminar_tema(temas[i]);
		free(temas[i]);
	}
	free(temas);

	printf("Temas eliminados; pulse para terminar: ");
	getchar();

	return 0;
}
