#include <mpi.h> 
#include <iostream>
#include<random>
#include<math.h>

using namespace std;

//#define DEBUG

void uso(string nombre_prog);

void obt_args(
	char*    argv[]        /* in  */,
	int&     quan         /*  in  */);

int main(int argc, char* argv[]) {
	random_device generator;
	uniform_real_distribution<double> distribution(-1.0, 1.0);
	double x, y, distancia, estimado, t1, t2;
	double local_start, local_finish, local_elapsed, elapsed;
	int aciertos = 0;
	int quan, block, local_int, total_int;
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
	MPI_Status mpi_status; // para capturar estado al finalizar invocación de funciones MPI

						   /* Arrancar ambiente MPI */
	MPI_Init(&argc, &argv);             		/* Arranca ambiente MPI */
	MPI_Comm_rank(MPI_COMM_WORLD, &mid); 		/* El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &cnt_proc);  /* El comunicador le da valor a p (número de procesos) */

#  ifdef DEBUG 
	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD);
#  endif

	/* ejecución del proceso principal */
	if (mid == 0) {
		uso("Aproximacion de Pi");
	}
	MPI_Barrier(MPI_COMM_WORLD);
	local_start = MPI_Wtime();
	obt_args(argv, quan);
	block = quan / cnt_proc;
	for (int i = mid*block; i < (mid*block)+block-1; i++) {
		x = distribution(generator);
		y = distribution(generator);
		distancia = x * x + y * y;
		if (distancia <= 1) {
			aciertos++;
			local_int = aciertos;
		}
	}
	MPI_Reduce(&local_int, &total_int, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	local_finish = MPI_Wtime();
	local_elapsed = local_finish - local_start;
	MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	if(mid == 0){
		estimado = 4 * (double)total_int / (double)quan;
		cout << "Tiempo transcurrido = " << elapsed << endl;
		cout << estimado << " Estimado con MonteCarlo" << endl;
		cout << 4.0*atan(1.0) << " Estimado con formula" << endl;
	}
	/* finalización de la ejecución paralela */
	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD); // para sincronizar la finalización de los procesos

	MPI_Finalize();
	return 0;
}  /* main */

   /*---------------------------------------------------------------------
   * REQ: N/A
   * MOD: N/A
   * EFE: despliega mensaje indicando cómo ejecutar el programa y pasarle parámetros de entrada.
   * ENTRAN:
   *		nombre_prog:  nombre del programa
   * SALEN: N/A
   */
void uso(string nombre_prog /* in */) {
	cerr << nombre_prog.c_str() << " secuencia de parámetros de entrada" << endl;
	cout << "Los parametros de entrada son la cantidad de procesos y de numeros a sumar" << endl;
	cout << "La salida son los estimados de MonteCarlo y por formula. Además, se despliega el tiempo que tardó el programa" << endl;
}  /* uso */

   /*---------------------------------------------------------------------
   * REQ: N/A
   * MOD: dato_salida
   * EFE: obtiene los valores de los argumentos pasados por "línea de comandos".
   * ENTRAN:
   *		nombre_prog:  nombre del programa
   * SALEN:
   *		dato_salida: un dato de salida con un valor de argumento pasado por "línea de comandos".
   */
void obt_args(
	char*    argv[]        /* in  */,
	int&     quan         /*  in  */) {

	quan = strtol(argv[1], NULL, 10); // se obtiene valor del argumento 2 pasado por "línea de comandos".

#  ifdef DEBUG
	cout << "dato_salida = " << quan << endl;

#  endif
}  /* obt_args */
