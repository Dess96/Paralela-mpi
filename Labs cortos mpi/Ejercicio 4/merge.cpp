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
#include<stdlib.h>
#include<iostream>
#include<time.h>
#include<vector>
#include<algorithm>
using namespace std;

//#define DEBUG

void uso(string nombre_prog);

void obt_args(
	char*    argv[]        /* in  */,
	int&     cant  /* out */);

std::vector<int> arreglo;
void generate(int);
void mergeSort(int, int);
void merge(int, int);
void merge_v2(int, int, int);

int main(int argc, char* argv[]) {
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
	int cant;
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
	uso("MergeSort paralelo");
	obt_args(argv, cant);
	generate(cant);
	mergeSort(cant, cnt_proc);
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
	cout << "El parametro de entrada es la cantidad de numeros a ordenar" << endl;
	cout << "La salida es la lista de numeros aleatorios ordenada por mergeSort" << endl;
	exit(0);
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
	int&     cant  /* out */) {

	cant = strtol(argv[1], NULL, 10); // se obtiene valor del argumento 1 pasado por "línea de comandos".

#  ifdef DEBUG
	cout << "dato_salida = " << dato_salida << endl;
#  endif
}  /* obt_args */

void generate(int cant) {
	int random;
	srand(time(NULL));
	arreglo.resize(cant);
	for (int i = 0; i < cant; i++) {
		random = rand();
		arreglo[i] = random;
	}
}

void mergeSort(int cant, int cnt_proc) {
	int quantity = cant / cnt_proc;
	int rango;
	vector<int>::iterator it;
	for (int i = 0; i < cant; i++) {
		it = arreglo.begin() + (rango*quantity);
		if (it + quantity != arreglo.end()) {
			sort(it, it + quantity);
		}
	}
	merge(cant, quantity);
	merge_v2(cant, quantity, cnt_proc);
}

void merge(int cant, int quantity) {
	vector<vector<int>> vectors;
	int shift = quantity;
	for (int i = 0; i < cant; i++) {
		vector<int> temp;
		temp.resize(cant);
		vectors.push_back(temp);
	}
	vector<int>::iterator it1 = arreglo.begin();
	merge(it1, it1 + quantity, it1 + quantity, it1 + 2 * quantity, vectors[0].begin());
	vector<int>::iterator it2 = vectors[0].begin();
	for (int i = 1; i <= cant; i++) {
		if ((quantity * shift) < cant && (quantity * shift + quantity) <= cant) {
			merge(it2, it2 + quantity * shift, it1 + quantity * shift, it1 + quantity * shift + quantity, vectors[i].begin());
			it2 = vectors[i].begin();
			shift++;
			it1 = vectors[i].begin();
		}
	}
}

void merge_v2(int cant, int quantity, int cnt_proc) {
	vector<vector<int>> vectors;
	vector<int>::iterator it1 = arreglo.begin();
	vector<int>::iterator it2 = arreglo.begin();
	int half;
	int j = 0;
	int shift = quantity;
	for (int i = 0; i < cant; i++) {
		vector<int> temp;
		temp.resize(cant);
		vectors.push_back(temp);
	}
	for (int niveles = 2; niveles < cnt_proc; niveles = niveles * 2) {
		half = cnt_proc / 2;
		for (int i = 0; i < cant; i++) {
			merge(it1, it1 + shift, it2 + shift + 1, (it2 + (2 * shift)), vectors[i].begin());
			if ((arreglo.begin() + (2 * shift)) != arreglo.end()) {
				it1 = arreglo.begin() + 2 * shift;
			}
			else {
				it1 = vectors[j].begin();
				j++;
				it2 = vectors[j].begin();
				shift = 2 * quantity;
			}
		}
	}
}
