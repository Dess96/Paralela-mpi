#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <mpi.h> 
#include <vector>
#include <fstream>

using namespace std;

#define i4_huge 2147483647

typedef vector< vector< int > > T_vec_vec_int;
typedef vector< int > T_vec_int;
int cntVertices;

void distancias_dijkstra(const T_vec_vec_int& ohd, T_vec_int& mind);
void buscar_mas_cercano(int s, int e, const T_vec_int& mind, const vector< bool >& connected, int& d, int& v);
void actualizar_mind(int s, int e, int mv, const vector< bool > connected, const T_vec_vec_int ohd, T_vec_int& mind);
void leeAdyacencias(ifstream& ae, T_vec_vec_int& ohd, int& cntVertices, T_vec_int& mind);

int main(int argc, char **argv) {
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
	MPI_Status mpi_status; // para capturar estado al finalizar icntVerticesocación de funciones MPI

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
	string nombreArchivoEntrada = "gpequenyo.txt"; // formato *.txt, por ejemplo "grafo.txt
	ifstream archivoEntrada(nombreArchivoEntrada, ios::in);
	T_vec_int mind;
	T_vec_vec_int ohd;

	if (!archivoEntrada) { // operador ! sobrecargado
		if (mid == 0) {
			cout << "No se pudo abrir el archivo de entrada" << endl;
		}
		cin.ignore();
		return 1;
	}

	if (mid == 0) {
		cout << "INICIA DIJKSTRA_OPENMP" << endl;
	}
	// se inicializa la matriz de adyacencias:
	leeAdyacencias(archivoEntrada, ohd, cntVertices, mind);

	if (mid == 0) {
		for (int i = 0; i < cntVertices; i++) {
			for (int j = 0; j < cntVertices; j++) {
				if (ohd[i][j] == i4_huge) {
					cout << "  Inf";
				}
				else {
					cout << "  " << setw(3) << ohd[i][j];
				}
			}
			cout << endl;
		}
	}
	distancias_dijkstra(ohd, mind);

	if (mid == 0) {
		cout << "  Distancias mínimas a partir del nodo 0:" << endl;
		for (int i = 0; i < cntVertices; i++) {
			cout << "  " << setw(2) << i
				<< "  " << setw(2) << mind[i] << "\n";
		}
		// finalización:
		cout << endl;
		cout << "TERMINA DIJKSTRA_OPENMP" << endl;
	}
	/* finalización de la ejecución paralela */
	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD); // para sincronizar la finalización de los procesos

	MPI_Finalize();
	return 0;
}

void distancias_dijkstra(const T_vec_vec_int& ohd, T_vec_int& mind) {
	vector< bool > connected;
	int md;  
	int mv;  
	int my_first; 
	int my_id; 
	int my_last; 
	int my_md;
	int my_mv;
	int nth; 

	connected.resize(cntVertices, false);
	connected[0] = true;

	mind.resize(cntVertices, 0);
	for (int i = 0; i < cntVertices; i++) {
		mind[i] = ohd[0][i];
	}
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id); 		/* El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &nth);  /* El comunicador le da valor a p (número de procesos) */
	my_first = (my_id * cntVertices) / nth; // se calcula el límite inferior de las iteraciones que realizará cada hilo
	my_last = ((my_id + 1) * cntVertices) / nth - 1; // se calcula el límite superior de las iteraciones que realizará cada hilo
	if (my_id == 0) {
		cout << endl;
		cout << "  P" << my_id << ": La region paralela comienza con " << nth << " hilos." << endl;
		cout << endl;
		cout << "  P" << my_id << ":  Primero=" << my_first << "  Ultimo=" << my_last << endl;
	}

	for (int my_step = 1; my_step < cntVertices; my_step++) {
		md = i4_huge;
		mv = -1;
		buscar_mas_cercano(my_first, my_last, mind, connected, my_md, my_mv);
		if (my_md < md)	{
			md = my_md;
			mv = my_mv;
		}
		MPI_Barrier(MPI_COMM_WORLD); // para sincronizar la finalización de los procesos

		if (mv != -1) {
			connected[mv] = true;
			if (my_id == 0) {
				cout << "  P" << my_id << ": Conectando al nodo " << mv << endl;
			}
		}
		MPI_Barrier(MPI_COMM_WORLD); // para sincronizar la finalización de los procesos

		if (mv != -1) {
			actualizar_mind(my_first, my_last, mv, connected, ohd, mind);
		}
	}
	if (my_id == 0) {
		cout << endl;
		cout << "  P" << my_id << ": Saliendo de la region paralela" << endl;
	}
}

void buscar_mas_cercano(int s, int e, const T_vec_int& mind, const vector< bool >& connected, int& d, int& v) {
	d = i4_huge;
	v = -1;
	for (int i = s; i <= e; i++) {
		if (!connected[i] && mind[i] < d) {
			d = mind[i];
			v = i;
		}
	}
}

void actualizar_mind(int s, int e, int mv, const vector< bool > connected, const T_vec_vec_int ohd, T_vec_int& mind) {
	for (int i = s; i <= e; i++) {
		if (!connected[i]) {
			if (ohd[mv][i] < i4_huge) {
				if (mind[mv] + ohd[mv][i] < mind[i]) {
					mind[i] = mind[mv] + ohd[mv][i];
				}
			}
		}
	}
}

void leeAdyacencias(ifstream& ae, T_vec_vec_int& ohd, int& cntVertices, T_vec_int& mind) {
	int pe;
	char finLinea = ' ';

	ae >> cntVertices; // el primer número del archivo es la cantidad de vértices
	mind.resize(cntVertices, 0);
	ohd.resize(cntVertices, mind);

	ae.get(); // consume un blanco
	finLinea = ae.peek(); // intenta leer fin de línea

	for (int i = 0; i < cntVertices; i++) {
		for (int j = 0; j < cntVertices; j++) {
			if (i == j) {
				ohd[i][i] = 0;
			}
			else {
				ohd[i][j] = i4_huge;
			}
		}
	}
	ae >> pe;
	for (int i = 0; i < cntVertices; i++) {
		for (int j = 0; j < cntVertices; j++) {
			if (!ae.eof() && (finLinea != '\n')) { // 10 ascii de fin de línea
				if (pe != -1) {
					ohd[i][j] = pe;
				}
			}
			ae >> pe;
			ae.get(); // consume un blanco
			finLinea = ae.peek(); // intenta leer fin de línea
		}
	}
}
