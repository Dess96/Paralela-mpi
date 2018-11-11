#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <mpi.h> 
#include <iostream>
#include <vector>
using namespace std;

# define NV 6
#define i4_huge 2147483647

typedef vector< vector< int > > T_vec_vec_int;
typedef vector< int > T_vec_int;

void distancias_dijkstra(const T_vec_vec_int& ohd, T_vec_int& mind);
void buscar_mas_cercano(int s, int e, const T_vec_int& mind, const vector< bool >& connected, int& d, int& v);
void inicializar(T_vec_vec_int& ohd);
void actualizar_mind(int s, int e, int mv, const vector< bool > connected, const T_vec_vec_int ohd, T_vec_int& mind);

int main(int argc, char **argv) {
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
	T_vec_int mind; 
	mind.resize(NV, 0);
	T_vec_vec_int ohd;
	ohd.resize(NV, mind); 
	cout << "INICIA DIJKSTRA_OPENMP" << endl;

	// se inicializa la matriz de adyacencias:
	inicializar(ohd);

	// se imprime la matriz de adyacencias:
	for (int i = 0; i < NV; i++) {
		for (int j = 0; j < NV; j++) {
			if (ohd[i][j] == i4_huge)
			{
				cout << "  Inf";
			}
			else
			{
				cout << "  " << setw(3) << ohd[i][j];
			}
		}
		cout << endl;
	}
	distancias_dijkstra(ohd, mind);

	// impresión de resultados:
	cout << "  Distancias mínimas a partir del nodo 0:" << endl;
	for (int i = 0; i < NV; i++)
	{
		cout << "  " << setw(2) << i
			<< "  " << setw(2) << mind[i] << "\n";
	}

	// finalización:
	cout << endl;
	cout << "TERMINA DIJKSTRA_OPENMP" << endl;
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

	connected.resize(NV, false);
	connected[0] = true;

	mind.resize(NV, 0);
	for (int i = 0; i < NV; i++)
	{
		mind[i] = ohd[0][i];
	}
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id); 		/* El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &nth);  /* El comunicador le da valor a p (número de procesos) */
	my_first = (my_id       * NV) / nth; // se calcula el límite inferior de las iteraciones que realizará cada hilo
	my_last = ((my_id + 1) * NV) / nth - 1; // se calcula el límite superior de las iteraciones que realizará cada hilo
	cout << endl;
	cout << "  P" << my_id << ": La region paralela comienza con " << nth << " hilos." << endl;
	cout << endl;
	cout << "  P" << my_id << ":  Primero=" << my_first << "  Ultimo=" << my_last << endl;

	for (int my_step = 1; my_step < NV; my_step++) {
		md = i4_huge;
		mv = -1;
		buscar_mas_cercano(my_first, my_last, mind, connected, my_md, my_mv);
		if (my_md < md)	{
			md = my_md;
			mv = my_mv;
		}
		if (mv != -1) {
			connected[mv] = true;
			cout << "  P" << my_id << ": Conectando al nodo " << mv << endl;
		}
		if (mv != -1) {
			actualizar_mind(my_first, my_last, mv, connected, ohd, mind);
		}
	}
	cout << endl;
	cout << "  P" << my_id << ": Saliendo de la region paralela" << endl;
}

void buscar_mas_cercano(int s, int e, const T_vec_int& mind, const vector< bool >& connected, int& d, int& v) {
	d = i4_huge;
	v = -1;
	for (int i = s; i <= e; i++)
	{
		if (!connected[i] && mind[i] < d)
		{
			d = mind[i];
			v = i;
		}
	}
}

void inicializar(T_vec_vec_int& ohd) {
	for (int i = 0; i < NV; i++)
	{
		for (int j = 0; j < NV; j++)
		{
			if (i == j)
			{
				ohd[i][i] = 0;
			}
			else
			{
				ohd[i][j] = i4_huge;
			}
		}
	}

	ohd[0][1] = ohd[1][0] = 40;
	ohd[0][2] = ohd[2][0] = 15;
	ohd[1][2] = ohd[2][1] = 20;
	ohd[1][3] = ohd[3][1] = 10;
	ohd[1][4] = ohd[4][1] = 25;
	ohd[2][3] = ohd[3][2] = 100;
	ohd[1][5] = ohd[5][1] = 6;
	ohd[4][5] = ohd[5][4] = 8;
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