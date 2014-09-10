#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "editor.h"


int main(int argc, char *argv[]) {
	int i;
	int num_eventos;
	int num_temas;
	char **temas;
	char valor[16];

	if ((argc<3) || ((num_eventos=atoi(argv[1]))<1)) {
                fprintf(stderr, "Uso: %s num_eventos tema...\n", argv[0]);
                return 1;
        }

	num_temas = argc - 2;
	temas= &argv[2];

	for (i=0; i<num_eventos; i++) {
		sprintf(valor, "%d", i);
		if (generar_evento(temas[i%num_temas], valor)<0)
			fprintf(stderr, "Error generando evento para tema %s\n",
                                 temas[i%num_temas]);
	}

	return 0;
}
