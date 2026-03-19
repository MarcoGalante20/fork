#ifndef NARGS
#define NARGS 4
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define INDICE_NOMBRE_ARCHIVO 0
#define INDICE_BINARIO 1
#define CANT_NECESARIA_ARG 2


void liberar_lineas(char **paquete, size_t ultimo_indice)
{
	for (size_t i = 1; i <= ultimo_indice; i++) {
		free(paquete[i]);
		paquete[i] = NULL;
	}
}


void ejecutar_binario(char *argv[], char **paquete, size_t ultimo_indice)
{
	pid_t id_hijo = fork();
	if (id_hijo < 0) {
		printf("Error al realizar el fork\n");
		liberar_lineas(paquete, ultimo_indice);
		exit(-1);
	}

	if (id_hijo == 0) {
		// PROCESO HIJO

		int resultado = execvp(argv[INDICE_BINARIO], paquete);
		if (resultado == -1) {
			printf("Ocurrió un error al ejecutar el binario");
			exit(-1);
		}

		exit(0);
	} else {
		// PROCESO PADRE
		wait(NULL);
	}

	liberar_lineas(paquete, ultimo_indice);
}


void ejecutar_bucle(char *argv[])
{
	char *paquete[NARGS + 2];
	for (int i = 0; i < (NARGS + 2); i++) {
		paquete[i] = NULL;
	}

	paquete[INDICE_NOMBRE_ARCHIVO] = argv[INDICE_BINARIO];
	paquete[NARGS + 1] = NULL;
	char *linea = NULL;
	int char_leidos;
	size_t indice_paquete = 1;
	size_t largo = 0;

	while ((char_leidos = getline(&linea, &largo, stdin)) != -1) {
		if (linea[char_leidos - 1] == '\n')
			linea[char_leidos - 1] = '\0';

		paquete[indice_paquete] = linea;

		if (indice_paquete == NARGS) {
			ejecutar_binario(argv, paquete, indice_paquete);
			indice_paquete = 0;
		}

		indice_paquete++;
		linea = NULL;
		largo = 0;
	}

	if (indice_paquete > 1)
		ejecutar_binario(argv, paquete, indice_paquete - 1);

	free(linea);
}


int main(int argc, char *argv[])
{
	if (argc != CANT_NECESARIA_ARG) {
		printf("Se recibió una cantidad inválida de argumentos\n");
		printf("Ejecutar el programa con el siguient formato: $ ./xargs <comando>\n");
		exit(-1);
	}

	ejecutar_bucle(argv);

	return 0;
}
