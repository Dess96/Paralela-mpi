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
#include<list>
using namespace std;

//#define DEBUG

void uso(string nombre_prog);

void obt_args(
	char*    argv[]        /* in  */,
	int&     num  /* out */);

int main(int argc, char* argv[]) {
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
	int num, sum, quan, block, odd, listSize, half, prime, block2;
	int r2 = 0;
	int r1 = 0;
	bool isSum = false;
	list<int> primes;
	list<int>::iterator it;
	list<int>::iterator it2;
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
	if (mid == 0) {
		uso("Conjetura de Goldbach");
	}
	obt_args(argv, num);
	half = cnt_proc / 2;
	block = num / half;
	odd = 3;
	/* ejecución del proceso principal */
	for (int i = mid * block; i < (mid*block) + block - 1; i++) {
		quan = 0;
		prime = 1;
		for (int j = mid * block; j < (mid*block) + block - 1; j++) {
			if (odd % prime == 0) {
				quan++;
			}
			if (quan == 2) {
				primes.push_back(prime);
			}
			prime++;
		}
		odd += 2;
	}
	listSize = primes.size();
	it = primes.begin();
	block2 = listSize / cnt_proc;
	for (int i = mid * block2; i < (mid*block2) + block2 - 1; i++) {
		it2 = it;
		if (!isSum) {
			for (int j = i; j < listSize; ++j) {
				sum = (*it) + (*it2);
				if (sum == num) {
					r1 = (*it);
					r2 = (*it2);
					isSum = true;
				}
				++it2;
			}
		}
		else {
			i = listSize;
		}
		++it;
	}
	if (mid == 0) {
		cout << "Los numeros que componen " << num << " son " << r1 << " y " << r2 << endl;
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