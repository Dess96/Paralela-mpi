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
	int num, quan, block, diff;
	int ind = 0;
	int r1 = 0;
	int r2 = 0;
	int r3 = 0;
	int send[3];
	int rec[3];
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
	int* primes = new int[num];
	/* ejecución del proceso principal */
	block = num / cnt_proc;
	for (int i = mid * block; i < (mid*block) + block - 1; i++) { //Llenamos una lista con numeros primos
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
	for (int i = mid * block; i < (mid*block) + block; i++) { //Ciclo principal
		if (i > 5) {
			if (i % 2 != 0) { //Los impares estan compuestos por tres primos
				for (int j = 0; j < num; j++) {
					for (int k = 0; k < num; k++) {
						diff = i - (primes[j] + primes[k]);
						for (int l = 0; primes[l] <= i / 2; l++) {
							if (diff == primes[k]) {
								r1 = primes[j];
								r2 = primes[k];
								r3 = diff;
							}
						}
					}
				}
			}
			else { //Los pares estan compuestos por dos primos
				for (int j = 0; j < num; j++){
					diff = i - primes[j];
					for (int k = 0; primes[k] <= i / 2; k++) {
						if (diff == primes[k]) {
							r1 = primes[j];
							r2 = diff;
							r3 = 0;
						}
					}
				}
			}
			if (mid == 0) {
				cout << "El numero " << i << " se forma por los numeros primos " << r1 << ", " << r2 << " y " << r3 << endl;
			}
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
