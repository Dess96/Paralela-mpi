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
	int buff;
	int r1 = 0;
	int r2 = 0;
	int r3 = 0;
	int r4 = 0;
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

	if (mid == 0) {
		uso("Conjetura de Goldbach");
	}
	obt_args(argv, num);
	int* primes = new int[num];
	int* send = new int[num * 4];
	int* rec = new int[num * 4];
	/* ejecuci�n del proceso principal */
	block = num / cnt_proc;
	for (int i = mid * block; i < (mid*block) + block; i++) { //Llenamos una lista con numeros primos
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

	buff = 0;
	for (int i = mid * block; i < (mid*block) + block; i++) { //Ciclo principal
		if (i > 5) {
			for (int j = 0; j < num; j++) {
				for (int k = 0; k < sqrt(num); k++) {
					diff = i - (primes[j] + primes[k]);
					for (int l = 0; primes[l] <= i / 2; l++) {
						if (diff == primes[k]) {
							r1 = i;
							r2 = primes[j];
							r3 = primes[k];
							r4 = diff;
						}
					}
				}
			}
			send[buff] = r1;
			buff++;
			send[buff] = r2;
			buff++;
			send[buff] = r3;
			buff++;
			send[buff] = r4;
			buff++;
		}
	}
	MPI_Gather(send, 20, MPI_INT, rec, num*4, MPI_INT, 0, MPI_COMM_WORLD);
	
	for (int i = 0; i < num * 4; i+=4) {
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