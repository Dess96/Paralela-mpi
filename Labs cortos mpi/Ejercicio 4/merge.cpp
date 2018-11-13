#include <mpi.h> 
#include <iostream>
#include<stdlib.h>
#include<time.h>
#include<vector>
#include<algorithm>
using namespace std;

//#define DEBUG

void uso(string nombre_prog);

void obt_args(
	char*    argv[]        /* in  */,
	int&     cant  /* out */);

void generate(int, vector<int>&);
void mergeSort(int, vector<int>&);
void merge(int, int, vector<int>&, int, int*);
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
	if (mid == 0) {
		uso("MergeSort");
	}
	obt_args(argv, cant);
	std::vector<int> arreglo;
	generate(cant, arreglo);
	mergeSort(cant, arreglo);
	/* finalización de la ejecución paralela */
	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD); // para sincronizar la finalización de los procesos. Sincronizacion (igual a openmp)

	MPI_Finalize();
	return 0;
}

void uso(string nombre_prog /* in */) {
	cerr << nombre_prog.c_str() << " secuencia de parametros de entrada" << endl;
	cout << "El parametro de entrada es la cantidad de numeros a ordenar" << endl;
	cout << "La salida es la lista de numeros aleatorios ordenada por mergeSort" << endl;
}  /* uso */

void obt_args(
	char*    argv[]        /* in  */,
	int&     cant  /* out */) {

	cant = strtol(argv[1], NULL, 10); // se obtiene valor del argumento 1 pasado por "línea de comandos".

#  ifdef DEBUG
	cout << "cant = " << cant << endl;
#  endif
}  /* obt_args */

void generate(int cant, vector<int>& arreglo) {
	int random;
	srand(time(NULL));
	arreglo.resize(cant);
	for (int i = 0; i < cant; i++) {
		random = rand();
		arreglo[i] = random;
	}
	MPI_Bcast(&arreglo[0], arreglo.size(), MPI_INT, 0, MPI_COMM_WORLD);
}

void mergeSort(int cant, vector<int>& arreglo) {
	int* send;
	int* rec = 0;
	int j = 0;
	int mid, cnt_proc, tam;
	MPI_Comm_rank(MPI_COMM_WORLD, &mid); 		/* El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &cnt_proc);  /* El comunicador le da valor a p (número de procesos) */
	int block = cant / cnt_proc;
	vector<int>::iterator it;
	send = (int*)malloc(block * sizeof(int));
	it = arreglo.begin() + (mid*block);
	sort(it, it + block); //Funciona correctamente
	for (int i = mid * block; i < (mid*block) + block; ++i) {
		send[j] = arreglo[i];
		j++;
	}
	tam = cnt_proc * block;
	if (mid == 0) {
		rec = (int*)malloc(tam * sizeof(int));;
	}
	MPI_Gather(send, block, MPI_INT, rec, block, MPI_INT, 0, MPI_COMM_WORLD);
	for (int i = 0; i < tam; i++) {
		if (mid == 0) {
			cout << rec[i] << endl;
		}
	}
	merge(cant, block, arreglo, mid, rec);
}

void merge(int cant, int quantity, vector<int>& arreglo, int mid, int* rec) {
	vector<vector<int>> vectors;
	int shift = quantity;
	for (int i = 0; i < cant; i++) {
		vector<int> temp;
		temp.resize(cant);
		vectors.push_back(temp);
	}
//	vector<int>::iterator it1 = arreglo.begin();
	int* it1 = &rec[0];
/*	if (mid == 0) {
		cout << " it1 " << *it1 << endl;
		it1 += quantity;
		cout << " it2 " << *it1 << endl;
		cout << " it3 " << *it1 + 2 * quantity << endl;
	}*/

	if (mid == 0) {
		merge(it1, it1 + quantity, it1 + quantity, it1 + 2 * quantity, vectors[0].begin());
	}
/*	vector<int>::iterator it2 = vectors[0].begin();
	for (int i = 1; i <= cant; i++) {
		if ((quantity * shift) < cant && (quantity * shift + quantity) <= cant) {
			merge(it2, it2 + quantity * shift, it1 + quantity * shift, it1 + quantity * shift + quantity, vectors[i].begin());
			it2 = vectors[i].begin();
			shift++;
			it1 = vectors[i].begin();
		}
	}*/
	for (int i = 0; i < vectors.size(); i++) {
		for (int j = 0; j < vectors[i].size(); j++) {
			cout << vectors[i][j] << " ";
		}
		cout << endl;
	}
}

/*void merge_v2(int cant, int quantity, int cnt_proc) {
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
}*/