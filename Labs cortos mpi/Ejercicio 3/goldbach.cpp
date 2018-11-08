/* Archivo:      mpi_plantilla.cpp
* Propósito:   ....
*
* Compilación:   mpicxx -g -Wall -o mpi_plantilla mpi_plantilla.cpp
* Ejecución:     mpiexec -n <num_proc> ./mpi_plantilla <secuencia de valores de parámetros>
*
* Entradas:     ...
* Salidas:    ...
*
* Notas:
* 1.  bandera DEBUG produce salida detallada para depuración.
*
*/

#include <mpi.h> 
#include <iostream>
#include<math.h>
#include<list>
#include<vector>
using namespace std;

//#define DEBUG

void uso(string nombre_prog);

void obt_args(
	char*    argv[]        /* in  */,
	int&     num  /* out */);

int main(int argc, char* argv[]) {
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
	int num, quan, block, diff, j, k, l;
	int ind = 0;
	bool isSum;
	MPI_Status mpi_status; // para capturar estado al finalizar invocación de funciones MPI
	vector<int> primes;

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
		uso("Conjetura de Goldbach");
	}
	obt_args(argv, num);

	int* send;
	block = num / cnt_proc;
	send = (int*)malloc(block * 4 * sizeof(int));
	primes.resize(num);
	for (int i = 2; i < num; i++) { //Llenamos una lista con numeros primos
		quan = 0;
		for (int j = 1; j <= i; j++) {
			if (i%j == 0) {
				quan++;
			}
		}
		if (quan == 2) {
			primes[ind] = i;
			ind++;
		}
	}
	int buff = 0;

	for (int i = mid * block; i < (mid*block) + block; ++i) { //Ciclo principal
		isSum = false;
		if (i > 5) {
			j = 0;
			while(j < i/2 && !isSum) {
				k = 0;
				while(k < i/2 && !isSum){
					diff = i - (primes[j] + primes[k]);
					l = 0;
					while(l < i/2 && !isSum){
						if (diff == primes[l] && !isSum) {
							isSum = true;
							send[buff] = i;
							buff++;
							send[buff] = primes[j];
							buff++;
							send[buff] = primes[k];
							buff++;
							send[buff] = diff;
							buff++;
						}
						l++;
					}
					k++;
				}
				j++;
			}
		}
	}

	int m = 0;
	int *rec = 0;
	int tam = cnt_proc * block * 4;
	if (mid == 0) {
		rec = (int*)malloc(tam * sizeof(int));;
	}
	MPI_Gather(send, block * 4, MPI_INT, rec, block * 4, MPI_INT, 0, MPI_COMM_WORLD);


	for (int i = 0; i < tam; i += 4) {
		if (mid == 0) {
			cout << "El numero " << rec[i] << " esta compuesto por " << rec[i + 1] << ", " << rec[i + 2] << " y " << rec[i + 3] << endl;
		}
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
	cerr << nombre_prog.c_str() << " secuencia de parametros de entrada" << endl;
	cout << "Los parametros de entrada son la cantidad de hilos y el n al que le vamos a calcular el numero Goldbach" << endl;
	cout << "La salida es la secuencia de numeros que componen la suma" << endl;
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
	int&     num  /* out */) {

	num = strtol(argv[1], NULL, 10); // se obtiene valor del argumento 1 pasado por "línea de comandos".

#  ifdef DEBUG
	cout << "dato_salida = " << num << endl;
#  endif
}  /* obt_args */
