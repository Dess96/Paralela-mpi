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
	int num, quan, it, it2, it3, sum;
	int ind = 1;
	int r1 = 0;
	int r2 = 0;
	int r3 = 0;
	bool isSum;
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

	uso("Conjetura de Goldbach");
	obt_args(argv, num);
	int* primes = new int[num];
	/* ejecución del proceso principal */
	primes[0] = 1;
	for (int i = 1; i < num; i++) { //Llenamos una lista con numeros primos
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
	for (int i = 5; i < num; i++) { //Ciclo principal
		isSum = false; //No hemos encontrado la suma
		if (i % 2 != 0) { //Los impares estan compuestos por tres primos
			it = 0;
			while (it < i && !isSum) {
				it2 = it;
				while (it2 < i && !isSum) {
					it3 = it2;
					while (it3 < i && !isSum) {
						sum = primes[it] + primes[it2] + primes[it3];
						if (sum == i) {
							r1 = primes[it];
							r2 = primes[it2];
							r3 = primes[it3];
							isSum = true;
						}
						it3++;
					}
					it2++;
				}
				it++;
			}
			cout << "El numero " << i << " se forma por los numeros primos " << r1 << ", " << r2 << " y " << r3 << endl;
		}
		else { //Los pares estan compuestos por dos primos
			it = 0;
			while (it < i && !isSum) {
				it2 = it;
				while (it2 < i && !isSum) {
					sum = primes[it] + primes[it2];
					if (sum == i) {
						r1 = primes[it];
						r2 = primes[it2];
						isSum = true;
					}
					it2++;
				}
				it++;
			}
			cout << "El numero "  << i << " se forma por los numeros primos " << r1 << " y " << r2 << endl;
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
