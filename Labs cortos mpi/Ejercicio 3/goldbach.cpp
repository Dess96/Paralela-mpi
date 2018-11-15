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

void obt_args(char* argv[], int& num);

void find(int diff, int num, int* send, int& buff, int i, int n);

vector<int> primes;

int main(int argc, char* argv[]) {
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
	int diff, num, quan, block;
	int buff = 0;
	int ind = 0;
	double local_start, local_finish, local_elapsed, elapsed;
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
		uso("Conjetura de Goldbach");
	}
	obt_args(argv, num);
	int *rec = 0;
	int* send;
	int tam;
	int n2;
	int diff2;
	int it3;
	int vecTam;
	block = num / cnt_proc;
	send = (int*)malloc(block * 4 * sizeof(int));
	tam = cnt_proc * block * 4;
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
	vecTam = ind--;
	MPI_Barrier(MPI_COMM_WORLD);
	local_start = MPI_Wtime();

	for (int i = mid * block; i < (mid*block) + block; ++i) { //Ciclo principal
		if (i > 5) {
			diff = i - 2;
			if (diff % 2 == 0) {
				int it2 = vecTam;
			    n2 = primes[it2];
				bool isPrime = false;
				while ((it2 >= 0) && !isPrime) {
					diff2 = diff - n2;
					it3 = 0;
					while (it3 < num / 2 && !isPrime) {
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
						else {
							it3++;
						}
					}
					if (!isPrime) {
						--it2;
						n2 = primes[it2];
					}
				}
			} else {
				send[buff] = i;
				buff++;
				send[buff] = diff2;
				buff++;
				send[buff] = 3;
				buff++;
				send[buff] = n2;
				buff++;
			}
		}
	}

	if (mid == 0) {
		rec = (int*)malloc(tam * sizeof(int));;
	}
	MPI_Gather(send, block * 4, MPI_INT, rec, block * 4, MPI_INT, 0, MPI_COMM_WORLD);

	local_finish = MPI_Wtime();
	local_elapsed = local_finish - local_start;
	MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	if (mid == 0)
		cout << "Tiempo transcurrido = " << elapsed << endl;

/*	for (int i = 0; i < tam; i += 4) {
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
}

void uso(string nombre_prog) {
	cerr << nombre_prog.c_str() << " secuencia de parametros de entrada" << endl;
	cout << "Los parametros de entrada son la cantidad de hilos y el n al que le vamos a calcular el numero Goldbach" << endl;
	cout << "La salida es la secuencia de numeros que componen la suma" << endl;
}

void obt_args(
	char*    argv[],
	int&     num) {

	num = strtol(argv[1], NULL, 10); // se obtiene valor del argumento 1 pasado por "línea de comandos".

#  ifdef DEBUG
	cout << "dato_salida = " << num << endl;
#  endif
}
