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
#include <stdio.h> 
#include <limits.h> 
#include <vector>
#include <fstream>

using namespace std;

//#define DEBUG

void uso(string nombre_prog);

void leeAdyacencias(ifstream& ae, vector< vector< int > >& ma, int& cntVertices);

int minDistance(int* dist, bool* sptSet, int cntVertices);

void printSolution(int* dist, int n);

void dijkstra(vector<vector<int>>, int src, int cntVertices);

int main(int argc, char* argv[]) {
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
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
	string nombreArchivoEntrada = "gpequenyo.txt"; // formato *.txt, por ejemplo "grafo.txt
	ifstream archivoEntrada(nombreArchivoEntrada, ios::in);
	vector< vector< int > > matrizAdyacencias;
	int cntVertices;

	if (mid == 0) {
		uso("Dijkstra");
	}

	if (!archivoEntrada) { // operador ! sobrecargado
		cout << "No se pudo abrir el archivo de entrada" << endl;
		cin.ignore();
		return 1;
	}
	leeAdyacencias(archivoEntrada, matrizAdyacencias, cntVertices);

	dijkstra(matrizAdyacencias, 0, cntVertices);
	/* finalización de la ejecución paralela */
	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD); // para sincronizar la finalización de los procesos

	MPI_Finalize();
	return 0;
}  /* main */

void uso(string nombre_prog /* in */) {
	cerr << nombre_prog.c_str() << " secuencia de parametros de entrada" << endl;
	cout << "Los parametros de entrada son la cantidad de nodos del grafo y la matriz de adyacencias de tamaño NxN (del nodo 0 al nodo N-1)" << endl;
	cout << "Las salidas del programa son las longitudes de los caminos mas cortos y la secuencia de nodos que los componen" << endl;
}  /* uso */

void leeAdyacencias(ifstream& ae, vector< vector< int > >& ma, int& cntVertices) {
	int pe;
	char finLinea = ' ';

	ae >> cntVertices; // el primer número del archivo es la cantidad de vértices
	vector< int > v;
	v.resize(cntVertices, INT_MAX);
	ma.resize(cntVertices, v);

	ae.get(); // consume un blanco
	finLinea = ae.peek(); // intenta leer fin de línea

	for (int i = 0; i < cntVertices; i++) {
		ma[i][i] = 0;
	}

	ae >> pe;
	for (int i = 0; i < cntVertices; i++) {
		for (int j = 0; j < cntVertices; j++) {
			if (!ae.eof() && (finLinea != '\n')) { // 10 ascii de fin de línea
				if (pe != -1) {
					ma[i][j] = pe;
				}
			}
			ae >> pe;
			ae.get(); // consume un blanco
			finLinea = ae.peek(); // intenta leer fin de línea
		}	
	}
}

int minDistance(int* dist, bool* sptSet, int cntVertices) {
	int min = INT_MAX, min_index;
	for (int v = 0; v < cntVertices; v++) {
		if (sptSet[v] == false && dist[v] <= min) {
			min = dist[v], min_index = v;
		}
	}
	return min_index;
}

void printSolution(int* dist, int n) {
	printf("Vertex   Distance from Source\n");
	for (int i = 0; i < n; i++)
		printf("%d tt %d\n", i, dist[i]);
}

void dijkstra(vector<vector<int>> matrizAdyacencias, int src, int cntVertices) {
	int* dist = new int[cntVertices];    
	bool* sptSet = new bool[cntVertices]; 
	for (int i = 0; i < cntVertices; i++)
		dist[i] = INT_MAX, sptSet[i] = false;

	dist[src] = 0;

	for (int count = 0; count < cntVertices - 1; count++) {
		int u = minDistance(dist, sptSet, cntVertices);
		sptSet[u] = true;
		for (int v = 0; v < cntVertices; v++)
			if (!sptSet[v] && matrizAdyacencias[u][v] && dist[u] != INT_MAX
				&& dist[u] + matrizAdyacencias[u][v] < dist[v])
				dist[v] = dist[u] + matrizAdyacencias[u][v];
	}
	printSolution(dist, cntVertices);
}