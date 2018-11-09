/* Archivo:      mpi_plantilla.cpp
* Prop�sito:   ....
*
* Compilaci�n:   mpicxx -g -Wall -o mpi_plantilla mpi_plantilla.cpp
* Ejecuci�n:     mpiexec -n <num_proc> ./mpi_plantilla <secuencia de valores de par�metros>
*
* Entradas:     ...
* Salidas:    ...
*
* Notas:
* 1.  bandera DEBUG produce salida detallada para depuraci�n.
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
	int num, quan, block, j, k, l;
	double local_start, local_finish, local_elapsed, elapsed;
	int sum = 2;
	int ind = 0;
	bool isSum;
	MPI_Status mpi_status; // para capturar estado al finalizar invocaci�n de funciones MPI
	vector<int> primes;

	/* Arrancar ambiente MPI */
	MPI_Init(&argc, &argv);             		/* Arranca ambiente MPI */
	MPI_Comm_rank(MPI_COMM_WORLD, &mid); 		/* El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &cnt_proc);  /* El comunicador le da valor a p (n�mero de procesos) */

#  ifdef DEBUG 
	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD);
#  endif

	/* ejecuci�n del proceso principal */
	if (mid == 0) {
		uso("Conjetura de Goldbach");
	}
	MPI_Barrier(MPI_COMM_WORLD);
	local_start = MPI_Wtime();
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
	int it2, n2, it3, diff2;
	bool isPrime;
	int diff;

	for (int i = mid * block; i < (mid*block) + block; ++i) { //Ciclo principal
		isSum = false;
		if (i > 5) {	
			diff = i - 2;
			if (diff % 2 == 0) {
				it2 = 0;
				n2 = primes[it2];
				isPrime = false;
				while (n2 < diff && it2 < num && !isPrime) {
					diff2 = diff - n2;
					it3 = 0;
					while (it3 < num && !isPrime) {
						if (diff2 == primes[it3]) {
							isPrime = true;
							send[buff] = i;
							buff++;
							send[buff] = diff2;
							buff++;
							send[buff] = 2;
							buff++;
							send[buff] = n2;
							buff++;
						}
						++it3;
					}
					if (!isPrime) {
						it2++;
						n2 = primes[it2];
					}
				}
			}
			else {
				diff = i - 3;
				it2 = 0;
				n2 = primes[it2];
				isPrime = false;
				while (n2 < diff & it2 < num && !isPrime) {
					diff2 = diff - n2;
					it3 = 0;
					while (it3 < num && !isPrime) {
						if (diff2 == primes[it3]) {
							isPrime = true;
							send[buff] = i;
							buff++;
							send[buff] = diff2;
							buff++;
							send[buff] = 3;
							buff++;
							send[buff] = n2;
							buff++;

						}
						else {
							it3++;
						}
					}
					if (!isPrime) {
						it2++;
						n2 = primes[it2];
					}
				}
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

	local_finish = MPI_Wtime();
	local_elapsed = local_finish - local_start;
	MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

/*	if (mid == 0)
		cout << "Tiempo transcurrido = " << elapsed << endl;*/

	for (int i = 0; i < tam; i += 4) {
		if (mid == 0) {
			cout << "El numero " << rec[i] << " esta compuesto por " << rec[i + 1] << ", " << rec[i + 2] << " y " << rec[i + 3] << endl;
		}
	}
	/* finalizaci�n de la ejecuci�n paralela */
	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD); // para sincronizar la finalizaci�n de los procesos

	MPI_Finalize();
	return 0;
}  /* main */

   /*---------------------------------------------------------------------
   * REQ: N/A
   * MOD: N/A
   * EFE: despliega mensaje indicando c�mo ejecutar el programa y pasarle par�metros de entrada.
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
   * EFE: obtiene los valores de los argumentos pasados por "l�nea de comandos".
   * ENTRAN:
   *		nombre_prog:  nombre del programa
   * SALEN:
   *		dato_salida: un dato de salida con un valor de argumento pasado por "l�nea de comandos".
   */
void obt_args(
	char*    argv[]        /* in  */,
	int&     num  /* out */) {

	num = strtol(argv[1], NULL, 10); // se obtiene valor del argumento 1 pasado por "l�nea de comandos".

#  ifdef DEBUG
	cout << "dato_salida = " << num << endl;
#  endif
}  /* obt_args */
