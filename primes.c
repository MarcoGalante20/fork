#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

#define CANTIDAD_NECESARIA_ARG 2
#define INDICE_N 1
#define BASE 10
#define LECTURA 0
#define ESCRITURA 1


void verificar_resultado_lectura(int resultado)
{
	if (resultado < 0) {
		printf("Error en la lectura de uno de los filtros\n");
		exit(-1);
	}
}


void pasar_numeros(int *pipe_izq_fds, int *pipe_der_fds, int primo)
{
	int candidato;
	int res_pipe_der;
	int res_pipe_izq = 1;

	while ((res_pipe_izq = read(pipe_izq_fds[LECTURA], &candidato, sizeof(candidato))) != 0) {
		verificar_resultado_lectura(res_pipe_izq);

		if ((candidato % primo) != 0) {
			res_pipe_der = write(pipe_der_fds[ESCRITURA], &candidato, sizeof(candidato));
			if (res_pipe_der < 0) {
				printf("Error en la escritura de uno de los filtros\n");
				exit(-1);
			}
		}
	}

	close(pipe_izq_fds[LECTURA]);
	close(pipe_der_fds[ESCRITURA]);
}


void imprimir_primos_restantes(int *pipe_izq_fds)
{
	int primo_restante;
	int resultado = 1;

	while((resultado = read(pipe_izq_fds[LECTURA], &primo_restante, sizeof(primo_restante))) != 0) {
		verificar_resultado_lectura(resultado);
		printf("primo %d\n", primo_restante);
	}

	close(pipe_izq_fds[LECTURA]);
}


void ejecutar_filtro(int *pipe_izq_fds, long n)
{
	close(pipe_izq_fds[ESCRITURA]);

	int primo;

	int resultado = read(pipe_izq_fds[LECTURA], &primo, sizeof(primo));
	if (resultado == 0) {
		close(pipe_izq_fds[LECTURA]);
		exit(0);
	}
	verificar_resultado_lectura(resultado);

	printf("primo %d\n", primo);
	if (primo > (n / primo)) {
		imprimir_primos_restantes(pipe_izq_fds);
		return;
	}

	int pipe_der_fds[2];
	if (pipe(pipe_der_fds) < 0) {
		printf("Error al crear un pipe derecho\n");
		exit(-1);
	}

	pid_t id_hijo = fork();
	if (id_hijo < 0) {
		printf("Error al realizar el fork en uno de los filtros\n");
		exit(-1);
	}

	if (id_hijo == 0) {
		// PROCESO HIJO
		close(pipe_izq_fds[LECTURA]);
		ejecutar_filtro(pipe_der_fds, n);
		exit(0);
	} else {
		// PROCESO PADRE
		close(pipe_der_fds[LECTURA]);
		pasar_numeros(pipe_izq_fds, pipe_der_fds, primo);
		wait(NULL);
	}
}


long obtener_numero(int argc, char *argv[])
{
	if (argc != CANTIDAD_NECESARIA_ARG) {
		printf("Cantidad de argumentos inválida\n");
		printf("Ejecutar el programa con la siguiente interfaz: $ ./primes <n>\n");
		exit(-1);
	}

	char *pos_final;
	long n = strtol(argv[INDICE_N], &pos_final, BASE);

	if ((n == LONG_MAX || n == LONG_MIN) && errno == ERANGE) {
		printf("El número recibido se excede del rango permitido\n");
		exit(-1);
	} else if (n < 2) {
		exit(0);
	} else if (pos_final == argv[INDICE_N]) {
		printf("El argumento contiene caracteres inválidos. Recuerde insertar un número entero\n");
		printf("Ejecutar el programa con la siguiente interfaz: $ ./primes <n>\n");
		exit(-1);
	}

	return n;
}


int main(int argc, char *argv[])
{
	long n = obtener_numero(argc, argv);

	int primer_pipe_fds[2];
	if (pipe(primer_pipe_fds) < 0) {
		printf("Error al crear el primer pipe\n");
		exit(-1);
	}

	int resultado;
	pid_t id_hijo = fork();

	if (id_hijo < 0) {
		printf("Error al realizar el fork\n");
		exit(-1);
	}

	if (id_hijo == 0) {
		// PROCESO HIJO
		ejecutar_filtro(primer_pipe_fds, n);
		exit(0);
	} else {
		// PROCESO PADRE
		close(primer_pipe_fds[LECTURA]);

		int candidato;

		for (int i = 2; i <= n; i++) {
			candidato = i;
			resultado = write(primer_pipe_fds[ESCRITURA], &candidato, sizeof(candidato));
			if (resultado < 0) {
				printf("Error en la escritura del primer proceso\n");
				exit(-1);
			}
		}

		close(primer_pipe_fds[ESCRITURA]);
		wait(NULL);
	}
}
