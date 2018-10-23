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
	int aciertos = 0;
	int quan;
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
	obt_args(argv, quan);
	//*************************************************************************
	t1 = MPI_Wtime();
	for (int i = 0; i < quan; i++) {
		x = distribution(generator);
		y = distribution(generator);
		distancia = x * x + y * y;
		if (distancia <= 1) {
			//Conflictos para modificar variable aciertos
			aciertos++;
		}
	}
	estimado = 4 * aciertos / quan;
	t2 = MPI_Wtime();
	cout << estimado << " Estimado con MonteCarlo" << endl;
	cout << 4.0*atan(1.0) << " Estimado con formula" << endl;
	cout << "El programa tardo " << t2 - t1 <<" segundos"<< endl;
	//**************************************************************************
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
	int&     quan         /*  in  */){

	quan = strtol(argv[1], NULL, 10); // se obtiene valor del argumento 2 pasado por "línea de comandos".

#  ifdef DEBUG
	cout << "dato_salida = " << quan << endl;

#  endif
}  /* obt_args */

