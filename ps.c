#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAMANIO_BUFFER 267
const char LECTURA[] = "r";

bool verificar_si_es_proceso(struct dirent *entrada)
{
	for (int i = 0; entrada->d_name[i] != '\0'; i++) {
		if (isdigit(entrada->d_name[i]) == 0)
			return false;
	}

	return true;
}


int main()
{
	DIR *directorio = opendir("/proc/");
	if (directorio == NULL) {
		printf("Ocurrió un error abriendo el directorio de procesos\n");
		exit(-1);
	}

	errno = 0;
	struct dirent *entrada;

	printf("    PID COMMAND\n");

	while ((entrada = readdir(directorio)) != NULL) {
		bool es_proceso = verificar_si_es_proceso(entrada);
		if (!es_proceso)
			continue;

		char ruta_comm[TAMANIO_BUFFER];
		snprintf(ruta_comm, sizeof(ruta_comm), "/proc/%s/comm", entrada->d_name);

		FILE *archivo = fopen(ruta_comm, LECTURA);
		if (archivo == NULL) {
			printf("Ocurrio un error al abrir el archivo con el nombre del proceso\n");
			continue;
		}

		char nombre_proceso[TAMANIO_BUFFER];

		if (fgets(nombre_proceso, sizeof(nombre_proceso), archivo) == NULL) {
			printf("Ocurrió un error al leer el archivo con el nombre del proceso\n");
			fclose(archivo);
			continue;
		}

		printf("%7s %s", entrada->d_name, nombre_proceso);
		fclose(archivo);
	}

	closedir(directorio);

	if (errno != 0) {
		printf("Ocurrió un error al acceder a los archivos del directorio de procesos\n");
		exit(-1);
	}

	return 0;
}
