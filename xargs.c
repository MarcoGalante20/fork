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

void liberar_lineas(char **paquete, size_t cant_elementos)
{
	for (size_t i = 1; i < cant_elementos; i++) {
		free(paquete[i]);
	}
}

void ejecutar_binario(char *argv[], char **paquete, size_t cant_elementos)
{
	pid_t id_hijo = fork();
	if (id_hijo < 0) {
		printf("Error al realizar el fork\n");
		liberar_lineas(paquete, cant_elementos);
		exit(-1);
	}

	if (id_hijo == 0) {
		// PROCESO HIJO
		int resultado = execvp(argv[INDICE_BINARIO], paquete);
		if (resultado == -1) {
			printf("Ocurrió un error al ejecutar el binario\n");
			liberar_lineas(paquete, cant_elementos);
			exit(-1);
		}
	} else {
		// PROCESO PADRE
		wait(NULL);
	}

	liberar_lineas(paquete, cant_elementos);
}


int main(int argc, char *argv[])
{
	if (argc != CANT_NECESARIA_ARG) {
		printf("Se recibió una cantidad inválida de argumentos\n");
		printf("Ejecutar el programa con el siguient formato: $ ./xargs <comando>\n");
		exit(-1);
	}

	char *linea = NULL;

	char *paquete[NARGS + 2];
	paquete[INDICE_NOMBRE_ARCHIVO] = argv[INDICE_NOMBRE_ARCHIVO];
	paquete[NARGS + 1] = NULL;
	size_t indice_paquete = 1;
	size_t largo = 0;
	int char_leidos;

	while ((char_leidos = getline(&linea, &largo, stdin)) != -1) {
		linea[char_leidos - 1] = '\0';
		paquete[indice_paquete] = linea;

		if ((indice_paquete + 1) == NARGS) {
			ejecutar_binario(argv, paquete, indice_paquete + 1);
			indice_paquete = 0;
		}

		indice_paquete++;
		linea = NULL;
	}
	if (indice_paquete > 1)
		ejecutar_binario(argv, paquete, indice_paquete + 1);

	free(linea);
	return 0;
}
