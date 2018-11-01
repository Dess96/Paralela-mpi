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
	int num, sum, r1, r2, quan;
	bool isSum = false;
	list<int> primes;
	list<int>::iterator it;
	list<int>::iterator it2;
	MPI_Status mpi_status; // para capturar estado al finalizar invocaci�n de funciones MPI

						   /* Arrancar ambiente MPI */
	MPI_Init(&argc, &argv);             		/* Arranca ambiente MPI */
	MPI_Comm_rank(MPI_COMM_WORLD, &mid); 		/* El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &cnt_proc);  /* El comunicador le da valor a p (n�mero de procesos) */

#  ifdef DEBUG 
	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD);
#  endif

	uso("Conjetura de Goldbach");
	obt_args(argv, num);
	/* ejecuci�n del proceso principal */
	for (int i = 3; i < num; i += 2) {
		quan = 0;
		for (int j = 1; j <= i; j++) {
			if (i % j == 0) {
				quan++;
			}
			if (quan == 2) {
				primes.push_back(j);
			}
		}
	}
	it = primes.begin();
	while (it != primes.end() && !isSum) {
		it2 = it;
		while (it2 != primes.end() && !isSum) {
			sum = (*it) + (*it2);
			if (sum == num) {
				r1 = (*it);
				r2 = (*it2);
				isSum = true;
			}
			it2++;
		}
		it++;
	}
	cout << "Los numeros que componen " << num << " son " << r1 << " y " << r2 << endl;
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