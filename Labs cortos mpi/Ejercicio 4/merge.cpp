#include<mpi.h> 
#include<iostream>
#include<stdlib.h>
#include<time.h>
#include<vector>
#include<algorithm>
using namespace std;

//#define DEBUG

void uso(string nombre_prog);
void obt_args(char* argv[], int& cant);
void generate(int, vector<int>&);
void mergeSort(int, vector<int>&);
void merge(int, int, vector<int>&, int, int*, int);
void merge_v2(int, int, int, vector<int>, int);
bool sender(int, int, int);
bool receiver(int, int, int);

int main(int argc, char* argv[]) {
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
	int cant;
	MPI_Status mpi_status; // para capturar estado al finalizar invocación de funciones MPI
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

void uso(string nombre_prog) {
	cerr << nombre_prog.c_str() << " secuencia de parametros de entrada" << endl;
	cout << "El parametro de entrada es la cantidad de numeros a ordenar" << endl;
	cout << "La salida es la lista de numeros aleatorios ordenada por mergeSort" << endl;
}

void obt_args(char* argv[], int& cant) {
	cant = strtol(argv[1], NULL, 10); // se obtiene valor del argumento 1 pasado por "línea de comandos".

#  ifdef DEBUG
	cout << "cant = " << cant << endl;
#  endif
}

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
	int mid, cnt_proc, tam, block;

	MPI_Comm_rank(MPI_COMM_WORLD, &mid); 		/* El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &cnt_proc);  /* El comunicador le da valor a p (número de procesos) */

	block = cant / cnt_proc;
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

	merge(cant, block, arreglo, mid, rec, cnt_proc);
	//merge_v2(cant, block, cnt_proc, arreglo, mid);
}

void merge(int cant, int quantity, vector<int>& arreglo, int mid, int* rec, int cnt_proc) {
	vector<vector<int>> vectors;
	int i = 1;
	int shift = quantity;
	for (int i = 0; i < cant; i++) {
		vector<int> temp;
		temp.resize(cant);
		vectors.push_back(temp);
	}
	int* it1 = &rec[0];

	if (mid == 0) {
		merge(it1, it1 + quantity, it1 + quantity, it1 + 2 * quantity, vectors[0].begin()); //Funciona correctamente
		if (cnt_proc > 2) {
			int* it2 = &vectors[0][0];
			while (!is_sorted(vectors[i - 1].begin(), vectors[i - 1].end())) {
				merge(it2, it2 + quantity * (i + 1), it1 + quantity * (i + 1), it1 + quantity * (i + 1) + quantity, vectors[i].begin());
				it2 = &vectors[i][0];
				++i;
			}
		}
	}
	if (mid == 0) {
		cout << "Rec despues de ordenar" << endl;
		for (int i = 0; i < vectors.size(); i++) {
			for (int j = 0; j < vectors[i].size(); j++) {
				cout << vectors[i][j] << " ";
			}
			cout << endl;
		}
	}
}

void merge_v2(int cant, int block, int cnt_proc, vector<int> arreglo, int mid) {
	int shift = 1;
	int shift2 = 1;
	int half = cnt_proc / 2;
	int iSend, iRec;
	int* send = (int*)malloc(cant * sizeof(int));
	int* rec = (int*)malloc(cant * sizeof(int));
	int* sendC = (int*)malloc(cant * sizeof(int));
	int* recC = (int*)malloc(cant * sizeof(int));
	int* it;
	int* it2;

	if (mid % 2 != 0) {
		iSend = mid - shift;
		send = &arreglo[mid*block];
		MPI_Send(send, block, MPI_INT, iSend, 0, MPI_COMM_WORLD);
	}
	else {
		iRec = mid + shift;
		MPI_Recv(rec, block, MPI_INT, iRec, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		it = &arreglo[mid*block];
		it2 = &rec[0];
		merge(it, it + block, it2, it2 + block, &sendC[0]);
	}
	MPI_Barrier(MPI_COMM_WORLD);

	while (half > 1) {
		shift *= 2;
		if (sender(mid, cnt_proc, shift)) {
			iSend = mid - shift;
			MPI_Send(sendC, shift * block, MPI_INT, iSend, 0, MPI_COMM_WORLD);
		}
		else if (receiver(mid, cnt_proc, shift)) {
			iRec = mid + shift;
			MPI_Recv(rec, shift * block, MPI_INT, iRec, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			it = &sendC[0]; //mi parte
			it2 = &rec[0]; //la parte que me envian
			merge(it, it + shift * block, it2, it2 + shift * block, &recC[0]);
			for (int i = 0; i < shift * block * 2; i++) {
				sendC[i] = recC[i];
			}
		}
		half /= 2;
		MPI_Barrier(MPI_COMM_WORLD);
	}
	if (mid == 0) {
		cout << "Proceso 0" << endl;
		for (int i = 0; i < shift * block; i++) {
			cout << recC[i] << " " << mid <<endl;
		}
	}
}

bool sender(int mid, int cnt_proc, int shift) {
	bool isSender = false;
	int i = 1;
	while (!isSender && i < cnt_proc) {
		if (mid == (shift * i)) {
			isSender = true;
		}
		else {
			i += 2;
		}
	}
	return isSender;
}

bool receiver(int mid, int cnt_proc, int shift) {
	bool isSender = false;
	int i = 0;
	while (!isSender && i < cnt_proc) {
		if (mid == (shift * i)) {
			isSender = true;
		}
		else {
			i += 2;
		}
	}
	return isSender;
}